/*
 * Copyright Institute for Theoretical Physics, ETH Zurich 2015.
 * Distributed under the Boost Software License, Version 1.0.
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef AMBIENT_CONTROLLERS_ACTOR_HPP
#define AMBIENT_CONTROLLERS_ACTOR_HPP

namespace ambient {

    // {{{ primary actor-class

    inline actor::~actor(){
        if(!this->controller) return;
        ambient::select().deactivate(this);
    }
    inline actor::actor(scope::const_iterator it){
        if(! (this->controller = ambient::select().activate(this)) ) return;
        this->round = this->controller->get_num_procs();
        this->rank = (*it) % this->round;
        this->state = (this->rank == controller->get_rank()) ? locality::local : locality::remote;
    }
    inline bool actor::remote() const {
        return (state == locality::remote);
    }
    inline bool actor::local() const {
        return (state == locality::local);
    }
    inline bool actor::common() const {
        return (state == locality::common);
    }
    inline rank_t actor::which() const {
        return this->rank;
    }

    // }}}
    // {{{ actor's special case: everyone does the same

    inline actor_common::actor_common(){
        if(! (this->controller = ambient::select().activate(this)) ){
            if(!ambient::select().get_actor().common()) throw std::runtime_error("Nested actor_common");
            return;
        }
        this->rank = controller->get_shared_rank();
        this->state = locality::common;
    }

    // }}}
    // {{{ actor's special case: auto-scheduling actor

    inline actor_auto::actor_auto(typename actor::controller_type* c){
        this->controller = c;
        this->controller->reserve();
        this->round = controller->get_num_procs();
        this->scores.resize(round, 0);
        this->set(0);
    }
    inline void actor_auto::set(scope::const_iterator it){
        this->set(*it);
    }
    inline void actor_auto::set(rank_t r){
        this->rank = r;
        this->state = (this->rank == controller->get_rank()) ? locality::local : locality::remote;
    }
    inline void actor_auto::intend_read(model::revision* r){
        if(r == NULL || model::common(r)) return;
        this->scores[model::owner(r)] += r->spec.extent;
    }
    inline void actor_auto::intend_write(model::revision* r){
        if(r == NULL || model::common(r)) return;
        this->stakeholders.push_back(model::owner(r));
    }
    inline void actor_auto::schedule(){
        int max = 0;
        rank_t rank = this->rank;
        if(stakeholders.empty()){
            for(int i = 0; i < this->round; i++)
            if(scores[i] >= max){
                max = scores[i];
                rank = i;
            }
        }else{
            for(int i = 0; i < stakeholders.size(); i++){
                rank_t k = stakeholders[i];
                if(scores[k] >= max){
                    max = scores[k];
                    rank = k;
                }
            }
            stakeholders.clear();
        }
        std::fill(scores.begin(), scores.end(), 0);
        this->set(rank);
    }

    // }}}
}

#endif
