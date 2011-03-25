#include "ambient/ambient.h"
#include "ambient/packets/types.h"
#include "ambient/packets/packet.h"
#include "ambient/packets/auxiliary.hpp"
#include "ambient/groups/packet_manager.h"
#include "ambient/groups/group.h"
#include "ambient/groups/auxiliary.hpp"
#include "ambient/core/layout.h"
#include "ambient/auxiliary.hpp"

#include "ambient/core/operation/operation.h"
#include "ambient/core/operation/operation.pp.sa.hpp"

#define AMBIENT_MASTER_RANK 0

using namespace ambient::packets; 
using namespace ambient::groups; 

namespace ambient
{
// global objects accessible anywhere //
    scheduler& layout       = scheduler::instance();
    scheduler& engine       = scheduler::instance();
    multirank& rank         = multirank::instance();
    hash_map& p_profile_map = hash_map::instance();
    scope_proxy& scope      = scope_proxy::instance();
// global objects accessible anywhere //

    scheduler & scheduler::operator>>(dim3 distr_dim) 
    {
        this->distr_dim = distr_dim;
        this->group_dim = NULL;
        this->gpu_dim = NULL;
        return *this;
    }
    scheduler & scheduler::operator,(dim3 dim) 
    {
        if(this->group_dim == NULL){
            this->group_dim = dim;
            this->default_data_packet_t = new block_packet_t(this->group_dim*this->item_dim); // to redo in future?
            this->default_data_packet_t->commit();
            if(!world()->get_manager()->subscribed(*this->default_data_packet_t)){
                world()->get_manager()->subscribe(*this->default_data_packet_t);
                world()->get_manager()->add_handler(*this->default_data_packet_t, new core::operation(accept_block, 
                    world()->get_manager()->get_pipe(*this->default_data_packet_t, packet_manager::IN)) );
            }
        }else if(this->gpu_dim == NULL){
            this->gpu_dim = dim;
        }
        return *this;
    }
    scheduler& operator>>(scheduler* instance, dim3 distr_dim) 
    {
        return *instance >> distr_dim;
    }
    scheduler& scheduler::instance()
    {
        static scheduler* singleton = NULL;
        if(!singleton) singleton = new scheduler();
        return *singleton;
    }
    void init(MPI_Comm comm){ engine.init(comm);  }
    void finalize()         { engine.finalize();  }
    void playout()          { engine.playout();   }
    void spin()             { engine.spin();      }
    void spin_loop()        { engine.spin_loop(); }
    int  size()             { return engine.size; }

    scheduler::scheduler(): item_dim(dim3(128,128,1)){ }
    dim3 scheduler::get_group_dim(){ return this->group_dim; }
    dim3 scheduler::get_item_dim() { return this->item_dim;  }
    dim3 scheduler::get_distr_dim(){ return this->distr_dim; }
    dim3 scheduler::get_gpu_dim()  { return this->gpu_dim;   }

    void scheduler::init(MPI_Comm comm)
    {
        int threading_level;
        this->comm = comm;
        if(this->comm == (MPI_Comm)NULL){
            MPI_Init_thread(0, NULL, MPI_THREAD_MULTIPLE, &threading_level);
            this->comm = MPI_COMM_WORLD;
        }
        MPI_Comm_size(this->comm, &this->size);

        this->ambient = new group("ambient", AMBIENT_MASTER_RANK, this->comm);
        this->default_data_packet_t = NULL;
// AUTO TUNING SHOULD START BELOW...

////////////////////////////////////
    }
    void scheduler::finalize()
    {
        MPI_Barrier(this->comm);
        MPI_Finalize();
    }
    void scheduler::push(core::operation* logistics, core::operation* computing)
    {
        this->stack.push_back(std::pair<core::operation*,core::operation*>(logistics,computing));
    }
    void scheduler::spin()
    {
        while(!this->router.alt_end_reached()){ // alt is for embedding
            (*this->router.alt_pick())->spin();
        }
    }
    void scheduler::spin_loop()
    {
        while(!this->router.end_reached()){
            (*this->router.pick())->spin_loop();
        }
    }
    void scheduler::playout()
    {
        one_touch_stack< std::pair<core::operation*,core::operation*> > filtered_stack;
        one_touch_stack< std::pair<core::operation*,core::operation*> > remaining_stack;
        std::pair<core::operation*, core::operation*>* pair;
        core::operation* logistics;
        core::operation* computing;

        core::operation* needle_op;
        core::operation* haystack_op;
// now let's iterate through the stack and mark dependencies
        while(!this->stack.end_reached()){
            printf("R%d: finding deps!\n", ambient::rank());
            needle_op = this->stack.pick()->first;
            for(int i=0; i < needle_op->count; i++){
                do{ haystack_op = this->stack.alt_pick()->first; 
                } while(needle_op != haystack_op); 
                while(!this->stack.alt_end_reached()){
                    printf("R%d: comparing with the next one!\n", ambient::rank());
                    haystack_op = this->stack.alt_pick()->first;
                    for(int j=0; j < haystack_op->count; j++)
                    if(needle_op->profiles[i] == haystack_op->profiles[j]){ // pointers comparison
                        printf("R%d: Added dep! %d eq %d\n", ambient::rank(), needle_op->profiles[i], haystack_op->profiles[j]);
                        needle_op->add_dependant(haystack_op);
                        break;
                    }
                }
            }
        }


        this->router.push_back(world()->get_manager());
        while(!this->stack.end_reached()){
            logistics = this->stack.pick()->first;
            logistics->perform();
            core::apply_changes(logistics->profiles, logistics->count);
            if(logistics->get_scope()->involved())
                this->router.push_back(logistics->get_scope()->get_manager());
        }
        this->spin_loop();
        while(!this->stack.end_reached()){
            pair = this->stack.pick();
            logistics = pair->first;
            if(logistics->get_scope()->involved()){
                computing = pair->second;
                computing->set_scope(logistics->get_scope());
                if(logistics->pin == NULL){ // nothing has been pinned
                    computing->invoke();    // scalapack style
                }else{
// performing computation for every item inside every appointed workgroup
                    std::vector<core::layout_table_entry> & workload = logistics->pin->layout->segment_count != 0 ? 
                                                                       logistics->pin->layout->segment : logistics->pin->layout->requests;
                    int workload_size = logistics->pin->layout->segment_count != 0 ? 
                                        logistics->pin->layout->segment_count : logistics->pin->layout->request_count; 
                    for(int k=0; k < workload_size; k++){
                        logistics->pin->set_default_group(workload[k].i, workload[k].j);
                        computing->invoke();
                        this->spin(); // processing any communications that did occur
                    }
                    logistics->finalize();
                }
            }
            this->spin_loop();
        }
// cleaning the layout
        while(!this->stack.end_reached()){
            logistics = this->stack.pick()->first;
            for(int i=0; i < logistics->count; i++)
                logistics->profiles[i]->clean();
        }
        this->stack.clean();
        this->router.clean();
    }

    bool is_master(){
        return ambient::rank.is_master();
    }
    group* world(){
        return engine.ambient;
    }

}
