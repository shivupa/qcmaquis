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

#ifndef AMBIENT_INTERFACE_ACCESS
#define AMBIENT_INTERFACE_ACCESS

namespace ambient {

    using ambient::models::ssm::revision;

    template <typename T> static bool exclusive(T& obj){
        controller.touch(obj.versioned.core);
        revision& c = *obj.versioned.core->current;
        if(controller.remote()){
            c.state = ambient::remote;
            c.owner = controller.which();
            return true;
        }else{
            c.state = ambient::local;
            if(!c.valid()) c.embed(get_allocator<T>::type::alloc(c.spec));
            return false;
        }
    }

    template <typename T> static revision& naked(T& obj){
        return *obj.versioned.core->current;
    }

    template <typename T> static revision& raw(T& obj){ 
        return *(revision*)obj.before;     
    }

    template <typename T> static typename T::unnamed::mapping& get(T& obj){ 
        controller.touch(obj.versioned.core);
        ambient::sync(); 
        revision& c = *obj.versioned.core->current;
        assert(c.state == ambient::local || c.state == ambient::common);
        if(!c.valid()){
            c.embed(get_allocator<T>::type::calloc(c.spec));
        }
        return *(typename T::unnamed::mapping*)c;
    }

    template <typename T> static typename T::unnamed::mapping& current(T& obj){
        revision& c = *(revision*)obj.before;
        if(!c.valid()){
            c.embed(get_allocator<T>::type::calloc(c.spec));
        }
        return *(typename T::unnamed::mapping*)c;
    }

    template <typename T> static typename T::unnamed::mapping& updated(T& obj){ 
        revision& c = *(revision*)obj.after; assert(!c.valid());
        revision& p = *(revision*)obj.before;
        if(c.valid()){ } // safety perk
        else if(p.valid() && p.locked_once() && !p.referenced() && c.spec.conserves(p.spec)) c.reuse(p);
        else{
            c.embed(get_allocator<T>::type::alloc(c.spec));
        }
        return *(typename T::unnamed::mapping*)c;
    }

    template <typename T> static typename T::unnamed::mapping& revised(T& obj){ 
        revision& c = *(revision*)obj.after; assert(!c.valid());
        revision& p = *(revision*)obj.before;
        if(!p.valid()){
            c.embed(get_allocator<T>::type::calloc(c.spec));
        }else if(p.locked_once() && !p.referenced() && c.spec.conserves(p.spec)) c.reuse(p);
        else{
            c.embed(get_allocator<T>::type::alloc(c.spec));
            memcpy((T*)c, (T*)p, p.spec.extent);
        }
        return *(typename T::unnamed::mapping*)c;
    }

    template <typename T> static typename T::unnamed::mapping& emptied(T& obj){
        typename T::unnamed::mapping& c = updated(obj);
        memset(&c, 0, ((revision*)obj.after)->spec.extent); 
        return c;
    }
}

#endif
