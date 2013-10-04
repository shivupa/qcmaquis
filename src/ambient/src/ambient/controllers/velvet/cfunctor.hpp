/*
 * Ambient, License - Version 1.0 - May 3rd, 2012
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

namespace ambient { namespace controllers { namespace velvet {

    // {{{ set revision

    inline set<revision>& set<revision>::spawn(revision& r){
        if(r.transfer == NULL){
            r.transfer = new (ambient::pool::malloc<bulk,set>())
                         set<revision>(r);
            ((set*)r.transfer)->clock = ambient::model.clock;
            ((set*)r.transfer)->sid = ambient::channel.index();
        }else if(((set*)r.transfer)->clock != ambient::model.clock){
            ((set*)r.transfer)->clock = ambient::model.clock;
            ((set*)r.transfer)->sid = ambient::channel.index();
        }
        return *(set<revision>*)r.transfer;
    }
    
    inline set<revision>::set(revision& r) : evaluated(false), target(&r), active(false) {
    }
    
    inline void set<revision>::operator >> (int p){
        if(active) return;
        this->active = true;
        if(p == AMBIENT_BROADCAST) p = ambient::rank.neighbor();
        handle = (request*)(size_t)p;
        if(target->generator != NULL) ((cfunctor*)target->generator)->queue(this);
        else{
            evaluated = true; // can send right now
            handle = ambient::channel.set(target, (size_t)handle, sid); 
            ambient::controller.queue(this);
        }
        target->use();
    }
    
    inline bool set<revision>::ready(){
        if(target->generator != NULL) return false;
        if(!evaluated){
            evaluated = true;
            handle = ambient::channel.set(target, (size_t)handle, sid); 
        }
        return ambient::channel.test(handle);
    }
    
    inline void set<revision>::invoke(){
        #ifdef AMBIENT_MEMORY_SQUEEZE
        ambient::controller.squeeze(target);
        #endif
        target->release(); 
        target->transfer = NULL;
    }

    // }}}
    // {{{ get revision

    inline void get<revision>::spawn(revision& r){
        if(r.transfer == NULL){
            if(ambient::rank() == 0) printf("T");
            r.transfer = new (ambient::pool::malloc<bulk,get>())
                              get<revision>(r);
            assist(r, ambient::rank());
            ((get<revision>*)r.transfer)->handle = ambient::channel.get(&r, ((assistance*)r.assist.second)->sid);
        }else
            assist(r, ambient::rank());
    }

    inline void get<revision>::assist(revision& r, int rank){
        if(r.assist.first != ambient::model.clock){
            r.assist.first = ambient::model.clock;
            r.assist.second = new (ambient::pool::malloc<bulk,assistance>()) assistance();
            ((assistance*)r.assist.second)->sid = ambient::channel.index();
        }
        *(assistance*)r.assist.second += rank;
    }

    inline get<revision>::get(revision& r)
    : target(&r)
    {
        target->generator = this;
        target->embed(ambient::pool::malloc<bulk>(target->spec));
        ambient::controller.queue(this);
        target->use();
    }
    
    inline bool get<revision>::ready(){
        if(ambient::channel.test(handle)){
            if(target->assist.first == ambient::model.clock){
                assistance* a = (assistance*)target->assist.second;
                if(a->handle >= 0)
                    handle = ambient::channel.set(target, a->handle, a->sid);
                target->assist.first--; // invalidate
            }
            //return true;
            return ambient::channel.test(handle);
        }
        return false;
    }

    inline void get<revision>::invoke(){
        #ifdef AMBIENT_MEMORY_SQUEEZE
        ambient::controller.squeeze(target);
        #endif
        target->release();
        target->complete();
        target->transfer = NULL;
    }

    // }}}
    // {{{ transformable broadcast get/set

    inline void get<transformable>::spawn(transformable& v, int owner){
        ambient::controller.queue(new get(v, owner));
    }
    inline get<transformable>::get(transformable& v, int owner)
    : target(&v) {
        sid = ambient::channel.index();
        handle = ambient::channel.get(target, sid);
    }
    inline bool get<transformable>::ready(){
        return ambient::channel.test(handle);
    }
    inline void get<transformable>::invoke(){}

    inline void set<transformable>::spawn(transformable& v, int owner){
        ((cfunctor*)v.generator)->queue(new set(v));
    }
    inline set<transformable>::set(transformable& v) 
    : target(&v) {
        sid = ambient::channel.index();
    }
    inline bool set<transformable>::ready(){
        if(target->generator != NULL) return false;
        if(handles.empty()){
            for(int i = 0; i < ambient::channel.wk_dim(); i++){
                if(i == ambient::rank()) continue;
                handles.push_back(ambient::channel.set(target, i, sid)); 
            }
        }
        bool r = true;
        for(int i = 0; i < handles.size(); i++){
            if(!ambient::channel.test(handles[i])) r = false;
        }
        return r;
    }
    inline void set<transformable>::invoke(){}

    // }}}

} } }

