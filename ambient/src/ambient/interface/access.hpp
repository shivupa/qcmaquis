/*
 * Copyright Institute for Theoretical Physics, ETH Zurich 2014.
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

#ifndef AMBIENT_INTERFACE_ACCESS
#define AMBIENT_INTERFACE_ACCESS

#define mapping typename T::ambient_desc::mapping

namespace ambient {

    using ambient::models::ssm::revision;

    template<typename V>
    inline void merge(const V& src, V& dst){
        assert(dst.ambient_rc.desc->current == NULL);
        if(weak(src)) return;
        revision* r = src.ambient_rc.desc->back();
        dst.ambient_rc.desc->current = r;
        // do not deallocate or reuse
        if(!r->valid() && r->state != ambient::locality::remote){
            assert(r->spec.region != region_t::delegated);
            r->spec.protect();
        }
        assert(!r->valid() || !r->spec.bulked() || ambient::models::ssm::model::remote(r)); // can't rely on bulk memory
        r->spec.crefs++;
    }

    template<typename V>
    inline void swap_with(V& left, V& right){
        std::swap(left.ambient_rc.desc, right.ambient_rc.desc);
        left.ambient_after = left.ambient_rc.desc->current;
        right.ambient_after = right.ambient_rc.desc->current;
    }

    template <typename T> static revision& naked(T& obj){
        return *obj.ambient_rc.desc->current;
    }

    template <typename T> static bool exclusive(T& obj){
        ambient::select().get_controller().touch(obj.ambient_rc.desc);
        revision& c = *obj.ambient_rc.desc->current;
        if(ambient::select().get_actor().remote()){
            c.state = ambient::locality::remote;
            c.owner = ambient::which();
            return true;
        }else{
            c.state = ambient::locality::local;
            if(!c.valid()) c.embed(get_allocator<T>::type::alloc(c.spec));
            return false;
        }
    }

    template <typename T> static T& load(T& obj){ 
        ambient::select().get_controller().touch(obj.ambient_rc.desc);
        ambient::sync(); 
        revision& c = *obj.ambient_rc.desc->current;
        assert(c.state == ambient::locality::local || c.state == ambient::locality::common);
        if(!c.valid()) c.embed(get_allocator<T>::type::calloc(c.spec));
        obj.ambient_after = obj.ambient_rc.desc->current;
        return obj;
    }

    template <typename T> static mapping& delegated(T& obj){
        return *(mapping*)(*obj.ambient_after);
    }

    template <typename T> static void transform(const T& obj){
        if(!is_polymorphic<T>::value) return;
        new ((void*)&obj) typename get_async_type<T>::type();
    }

    template <typename T> static void revise(const T& obj){
        revision& c = *obj.ambient_before; if(c.valid()) return;
        c.embed(get_allocator<T>::type::calloc(c.spec));
    }

    template <typename T> static void revise(volatile T& obj){
        revision& c = *obj.ambient_after; if(c.valid()) return;
        revision& p = *obj.ambient_before;
        if(p.valid() && p.locked_once() && !p.referenced() && c.spec.conserves(p.spec)) c.reuse(p);
        else c.embed(get_allocator<T>::type::alloc(c.spec));
    }

    template <typename T> static void revise(T& obj){
        revision& c = *obj.ambient_after; if(c.valid()) return;
        revision& p = *obj.ambient_before;
        if(!p.valid()) c.embed(get_allocator<T>::type::calloc(c.spec));
        else if(p.locked_once() && !p.referenced() && c.spec.conserves(p.spec)) c.reuse(p);
        else{
            c.embed(get_allocator<T>::type::alloc(c.spec));
            memcpy((T*)c, (T*)p, p.spec.extent);
        }
    }
}

#undef mapping
#endif
