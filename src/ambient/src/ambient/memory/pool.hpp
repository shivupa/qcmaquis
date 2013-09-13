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

#ifndef AMBIENT_MEMORY_POOL
#define AMBIENT_MEMORY_POOL

#include <sys/mman.h>
#include <boost/pool/singleton_pool.hpp>

namespace ambient { namespace memory {

    constexpr size_t paged(size_t size)  {  return PAGE_SIZE * (size_t)((size+PAGE_SIZE-1)/PAGE_SIZE); }
    constexpr size_t aligned(size_t size){  return ALIGNMENT * (size_t)((size+ALIGNMENT-1)/ALIGNMENT); }

    struct standard {
        static void* malloc(size_t sz){ return std::malloc(sz); }
        static void free(void* ptr){ std::free(ptr);  }
        static region_t signature(){
            return region_t::rstandard;
        }
    };

    struct fixed {
        // note: singleton_pool contains implicit mutex (critical for gcc)
        template<size_t S> static void* malloc(){ return std::malloc(S); } // boost::singleton_pool<fixed,S>::malloc(); 
        template<size_t S> static void free(void* ptr){ std::free(ptr);  } // boost::singleton_pool<fixed,S>::free(ptr);
    };

    struct bulk {

        template<size_t S>
        class factory {
        public:
            typedef boost::details::pool::default_mutex mutex;

            static factory& instance(){
                static factory singleton;
                return singleton;
            }
            factory(){
                this->buffers.push_back(std::malloc(S));
                this->counts.push_back(0);
                this->buffer = &this->buffers[0];
                this->reuse_count = 0;
            }
            static void* provide(){
                factory& s = instance();
                boost::details::pool::guard<mutex> g(s.mtx);
                void* chunk;

                if(s.r_buffers.empty()){
                    chunk = *s.buffer;
                    if(*s.buffer == s.buffers.back()){
                        s.buffers.push_back(std::malloc(S));
                        s.counts.push_back(0);
                        s.buffer = &s.buffers.back();
                    }else
                        s.buffer++;
                }else{
                    chunk = s.r_buffers.back();
                    s.r_buffers.pop_back();
                    s.reuse_count++;
                }

                return chunk;
            }
            static void collect(void* chunk, long int usage){
                factory& s = instance();
                boost::details::pool::guard<mutex> g(s.mtx);

                for(int i = 0; i < s.buffers.size(); i++){
                    if(s.buffers[i] == chunk){
                        s.counts[i] += usage;
                        if(s.counts[i] == 0) s.r_buffers.push_back(chunk);
                        break;
                    }
                }
            }
            static void reuse(void* ptr){
                factory& s = instance();
                boost::details::pool::guard<mutex> g(s.mtx);

                for(int i = 0; i < s.buffers.size(); i++){
                    if((size_t)ptr < ((size_t)s.buffers[i] + S) && (size_t)ptr >= (size_t)s.buffers[i]){
                        s.counts[i]--;
                        if(s.counts[i] == 0) s.r_buffers.push_back(s.buffers[i]);
                        break;
                    }
                }
            }
            static void reset(){
                factory& s = instance();
                s.buffer = &s.buffers[0];
                s.r_buffers.clear();
                for(int i = 0; i < s.counts.size(); i++)
                    s.counts[i] = 0;
                s.reuse_count = 0;
            }
            static size_t size(){
                factory& s = instance();
                return (s.buffer - &s.buffers[0]);
            }
            static size_t reused(){
                factory& s = instance();
                return s.reuse_count;
            }
            static size_t reserved(){
                factory& s = instance();
                return (s.counts.size());
            }
            mutex mtx;
            std::vector<long int> counts;
            std::vector<void*> buffers;
            std::vector<void*> r_buffers;
            size_t reuse_count;
            void** buffer;
        };

        template<size_t S>
        class region {
        public:
            typedef boost::details::pool::default_mutex mutex;

            region(){
                this->buffer = NULL;
                this->iterator = (char*)this->buffer+S;
                this->count = 0;
                this->block_count = 0;
            }
            void realloc(){
                if(this->count){
                     factory<S>::collect(this->buffer, this->count);
                     this->count = 0;
                }
                this->buffer = factory<S>::provide();
                this->iterator = (char*)this->buffer;
                this->block_count++;
            }
            void* malloc(size_t sz){
                boost::details::pool::guard<mutex> g(this->mtx);
                if(((size_t)iterator + sz - (size_t)this->buffer) >= S) realloc();
                void* m = (void*)iterator;
                iterator += aligned(sz);
                this->count++;
                return m;
            }
            void reset(){
                this->iterator = (char*)this->buffer+S;
                this->count = 0; 
                this->block_count = 0;
            }
            int block_count;
        private:
            void* buffer;
            char* iterator;
            long int count;
            mutex mtx;
        };

       ~bulk(){ 
            delete this->master; 
        }
        static bulk& instance(){
            static bulk singleton;
            return singleton;
        }
        bulk(){
            this->arity = ambient::get_num_threads();
            this->master = new region<AMBIENT_BULK_CHUNK>();
            this->slave  = new region<AMBIENT_BULK_CHUNK>();
        }
        template<size_t S> static void* malloc()         { return instance().master->malloc(S);  }
                           static void* malloc(size_t sz){ return instance().master->malloc(sz); }
                           static void reuse(void* ptr)  { factory<AMBIENT_BULK_CHUNK>::reuse(ptr); }
        template<size_t S> static void free(void* ptr)   { }
                           static void free(void* ptr)   { }
        static void* reserve(size_t sz){
            if(instance().slave->block_count < factory<AMBIENT_BULK_CHUNK>::reserved()/2)
                return instance().slave->malloc(sz);
            else 
                return instance().master->malloc(sz); 
        }

        static void report(){
            bulk& pool = instance();
            size_t pools = factory<AMBIENT_BULK_CHUNK>::size();
            if(pools > 2*pool.arity){
                std::cout << "R" << ambient::rank() << ": reused chunks: " << factory<AMBIENT_BULK_CHUNK>::reused() << "!\n";
                std::cout << "R" << ambient::rank() << ": reserved chunks: " << instance().slave->block_count << "!\n";
                std::cout << "R" << ambient::rank() << ": bulk memory: " << pools << " full chunks used\n";
                #ifdef AMBIENT_TRACE
                AMBIENT_TRACE
                #endif
            }
        }

        static void drop(){
            #ifdef AMBIENT_REPORT_BULK_USAGE
            report();
            #endif
            bulk& pool = instance();
            pool.master->reset();
            pool.slave->reset();
            factory<AMBIENT_BULK_CHUNK>::reset();
        }
        static region_t signature(){
            return region_t::rbulked;
        }
    private:
        region<AMBIENT_BULK_CHUNK>* master;
        region<AMBIENT_BULK_CHUNK>* slave;
        int arity;
    };

} }

namespace ambient {
    using memory::standard;
    using memory::fixed;
    using memory::bulk;

    namespace pool {
        struct descriptor {

            descriptor(size_t e, region_t r = region_t::rstandard) : extent(e), region(r), persistency(1), crefs(1), reusable(false) {}

            void zombie(){
                //region = region_t::rpersist;
            }
            void protect(){
                assert(region != region_t::rdelegated);
                if(!(persistency++)) region = region_t::rstandard;
            }
            void weaken(){
                assert(region != region_t::rbulked);
                assert(region != region_t::rdelegated);
                if(!(--persistency)) region = region_t::rbulked;
            }
            void reuse(descriptor& d){
                region   = d.region;
                d.region = region_t::rdelegated;
            }
            bool conserves(descriptor& p){
                assert(p.region != region_t::rdelegated && region != region_t::rdelegated);
                return (!p.bulked() || bulked());
            }
            bool bulked(){
                return (region == region_t::rbulked);
            }
            void reserve(){
                reusable = true;
            }

            size_t extent;
            region_t region;
            int persistency;
            int crefs;
            bool reusable;
        };

        template<class Memory>           static void* malloc(size_t sz){ return Memory::malloc(sz);            }
        template<class Memory, size_t S> static void* malloc()         { return Memory::template malloc<S>();  }
        template<class Memory, class  T> static void* malloc()         { return malloc<Memory, sizeof(T)>();   }
        template<class Memory>           static void free(void* ptr)   { return Memory::free(ptr);             }
        template<class Memory, size_t S> static void free(void* ptr)   { return Memory::template free<S>(ptr); }
        template<class Memory, class  T> static void free(void* ptr)   { return free<Memory, sizeof(T)>(ptr);  }

        template<class Memory>
        static void* malloc(descriptor& d){
            d.region = Memory::signature();
            return Memory::malloc(d.extent);
        }

        static void* malloc(descriptor& d){
            assert(d.region != region_t::rdelegated);
            if(d.region == region_t::rbulked){
                if(d.extent > AMBIENT_IB_EXTENT){
                    d.region = region_t::rstandard;
                    return malloc<standard>(d.extent);
                }
                if(d.reusable){
                    return ambient::memory::bulk::reserve(d.extent);
                }
                return malloc<bulk>(d.extent); 
            } else return malloc<standard>(d.extent);
        }
        static void free(void* ptr, descriptor& d){ 
            if(ptr == NULL || d.region == region_t::rdelegated) return;
            if(d.region == region_t::rbulked) free<bulk>(ptr);
            else free<standard>(ptr);
        }
    }
}

#endif
