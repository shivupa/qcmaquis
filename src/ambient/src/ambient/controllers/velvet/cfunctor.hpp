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

#define ALL -1

namespace ambient { namespace controllers { namespace velvet {

    // {{{ set revision

    template<int N>
    inline set<revision>& set<revision, N>::spawn(revision& r){
        if(r.transfer == NULL){
#ifdef AMBIENT_PERSISTENT_TRANSFERS
            r.transfer = new (ambient::pool::malloc<fixed,set>())
#else
            r.transfer = new (ambient::pool::malloc<bulk,set>())
#endif
                         set<revision>(r);
        }
        return *(set<revision>*)r.transfer;
    }
    
    template<int N>
    inline set<revision, N>::set(revision& r) : target(&r) {
#ifdef AMBIENT_PERSISTENT_TRANSFERS
        states = new std::vector<bool>(AMBIENT_MAX_NUM_PROCS, false);
#endif
    }
    
    template<int N>
    template<int NE>
    inline set<revision, N>::set(set<revision, NE>* s) 
    : evaluated(false), target(s->target),
      handle(s->handle)
    {
#ifdef AMBIENT_PERSISTENT_TRANSFERS
        states = s->states;
        if(N == AMBIENT_MAX_NUM_PROCS+1) handles = new std::vector<request*>();
#endif
    }
    
    template<int N>
    inline void set<revision, N>::operator >> (int p){
#ifndef AMBIENT_PERSISTENT_TRANSFERS
        if(N == AMBIENT_MAX_NUM_PROCS) return;
        new (this) set<revision, AMBIENT_MAX_NUM_PROCS>(this);
        if(p == ALL) p = ambient::neighbor();
        handle = (request*)(size_t)p;
        if(target->generator != NULL) ((cfunctor*)target->generator)->queue(this);
        else{
            evaluated = true; // can send right now
            handle = ambient::channel.set(target, (size_t)handle); 
            ambient::controller.queue(this);
        }
        target->use();
#else
        if(N == AMBIENT_MAX_NUM_PROCS){
            if(p == ALL){
                for(int i = 0; i < ambient::channel.dim(); ++i) (*states)[i] = true;
            }else
                (*states)[p] = true;
            return;
        }
        if(N == AMBIENT_MAX_NUM_PROCS+1){
            if(p == ALL){
                for(int i = 0; i < ambient::channel.dim(); ++i) (*this) >> i;
                return;
            }
            if((*states)[p]) return;
            (*states)[p] = true;
            if(!evaluated){
                ambient::controller.queue(this);
                target->use();
            }
            handles->push_back(ambient::channel.set(target, (size_t)p)); 
            evaluated = true;
        }else{
            new (this) set<revision, AMBIENT_MAX_NUM_PROCS>(this);
            if(p == ALL){
                for(int i = 0; i < ambient::channel.dim(); ++i) (*states)[i] = true;
                p = ambient::neighbor();
            }
            handle = (request*)(size_t)p;
            if(target->generator != NULL) ((cfunctor*)target->generator)->queue(this);
            else{
                evaluated = true; // can send right now
                handle = ambient::channel.set(target, (size_t)handle); 
                ambient::controller.queue(this);
            }
            target->use();
            (*states)[p] = true;
        }
#endif
    }
    
    template<int N>
    inline bool set<revision, N>::ready(){
        if(target->generator != NULL) return false;
        if(!evaluated){
            evaluated = true;
            handle = ambient::channel.set(target, (size_t)handle); 
        }
#ifndef AMBIENT_PERSISTENT_TRANSFERS
        return ambient::channel.test(handle);
#else
        if(N == AMBIENT_MAX_NUM_PROCS+1){
            bool result = true;
            for(int i = 0; i < handles->size(); ++i)
                if(!ambient::channel.test((*handles)[i])) result = false;
            if(!result) return false;
            delete handles;
            return true;
        }else{
                   ambient::channel.test(handle);
            return ambient::channel.test(handle);
        }
#endif
    }
    
    template<int N>
    inline void set<revision, N>::invoke(){
        target->release(); 
#ifdef AMBIENT_PERSISTENT_TRANSFERS
        new (this) set<revision, AMBIENT_MAX_NUM_PROCS+1>(this);
#else
        target->transfer = NULL;
#endif
    }

    // }}}
    // {{{ get revision

    inline pass<revision>::pass(revision& r, int rank) : target(&r) {
        target->use();
        handle = ambient::channel.set(target, rank); 
    }
    
    inline bool pass<revision>::ready(){
        return ambient::channel.test(handle);
    }
    
    inline void pass<revision>::invoke(){
        target->release(); 
    }

    inline void get<revision>::spawn(revision& r){
        if(r.transfer == NULL){
#ifdef AMBIENT_PERSISTENT_TRANSFERS
            r.transfer = new (ambient::pool::malloc<fixed,get>())
#else
            r.transfer = new (ambient::pool::malloc<bulk,get>())
#endif
                              get<revision>(r);
        }
        assist(r, ambient::rank());
    }

    inline void get<revision>::assist(revision& r, int rank){
        if(r.assist.first != ambient::model.clock){
            r.assist.first = ambient::model.clock;
            r.assist.second = new (ambient::pool::malloc<bulk,assistance>()) assistance();
        }
        *(assistance*)r.assist.second += rank;
    }

    inline get<revision>::get(revision& r)
    : target(&r) 
    {
        target->generator = this;
#ifdef AMBIENT_PERSISTENT_TRANSFERS
        target->embed(ambient::pool::malloc(target->spec));
#else
        target->embed(ambient::pool::malloc<bulk>(target->spec));
#endif
        handle = ambient::channel.get(target);
        ambient::controller.queue(this);
    }
    
    inline bool get<revision>::ready(){
        if(ambient::channel.test(handle)){
            if(target->assist.first == ambient::model.clock){
                assistance* a = (assistance*)target->assist.second;
                if(a->handle >= 0)
                    handle = ambient::channel.set(target, a->handle);
                target->assist.first--; // invalidate
            }
            //return true;
            return ambient::channel.test(handle);
        }
        return false;
    }

    inline void get<revision>::invoke(){
        target->complete();
#ifndef AMBIENT_PERSISTENT_TRANSFERS
        target->transfer = NULL;
#endif
    }

    // }}}
    // {{{ transformable broadcast get/set

    inline void get<transformable>::spawn(transformable& v, int owner){
        ambient::controller.queue(new get(v, owner));
    }
    inline void set<transformable, AMBIENT_MAX_NUM_PROCS>::spawn(transformable& v, int owner){
        ((cfunctor*)v.generator)->queue(new set(v));
    }

    inline get<transformable>::get(transformable& v, int owner)
    : target(&v), evaluated(false) {
        handle = ambient::channel.get(target);
        if(ambient::neighbor() == owner) evaluated = true;
    }
    inline set<transformable, AMBIENT_MAX_NUM_PROCS>::set(transformable& v) 
    : target(&v), handle(NULL) {
    }

    inline bool get<transformable>::ready(){
        if(ambient::channel.test(handle)){
            if(!evaluated){
                handle = ambient::channel.set(target, ambient::neighbor());
                evaluated = true;
            }
            return ambient::channel.test(handle);
        }
        return false;
    }
    inline bool set<transformable, AMBIENT_MAX_NUM_PROCS>::ready(){
        if(target->generator != NULL) return false;
        if(!handle) handle = ambient::channel.set(target, ambient::neighbor()); 
        return ambient::channel.test(handle);
    }

    inline void get<transformable>::invoke(){}
    inline void set<transformable, AMBIENT_MAX_NUM_PROCS>::invoke(){}

    // }}}

} } }

#undef ALL
