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

#ifndef AMBIENT_INTERFACE_SCOPE
#define AMBIENT_INTERFACE_SCOPE

namespace ambient { 

    class iscope {
    public:
        int rank;
        ambient::locality state;
        virtual bool tunable() const = 0;
        virtual void score(int c, size_t v) const {}
        virtual void select(int c) const {}
        virtual void schedule(){}
    };

    template<scope_t T = scope_t::single>
    class scope {};

    template<>
    class scope<scope_t::base> : public iscope {
    public:
        typedef controllers::ssm::controller controller_type;
        typedef typename controllers::ssm::controller::model_type model_type;
        controller_type c;

        scope();
        controller_type& get_controller(size_t n = AMBIENT_THREAD_ID); 
        void sync();
        bool scoped() const;
        
        void set_context(const iscope* s);
        void pop_context();

        bool remote() const;
        bool local() const;
        bool common() const;
        int which() const;

        virtual void intend_read(models::ssm::revision* o);
        virtual void intend_write(models::ssm::revision* o);
        virtual bool tunable() const ; 
        virtual void score(int c, size_t v) const ;
        virtual void select(int c) const ;
        virtual void schedule();
        mutable std::vector<int> stakeholders;
        mutable std::vector<int> scores;
        int round;

        const iscope* context;
    };

    #ifdef AMBIENT_BUILD_LIBRARY
    scope<scope_t::base> ctxt;
    void sync(){ ctxt.sync(); }
    #else
    extern scope<scope_t::base> ctxt;
    #endif

    template<>
    class scope<scope_t::threaded> : public iscope {
    public:
        scope(const std::vector<int>& map, int iterator = 0);
       ~scope();
        virtual bool tunable() const ;
        bool dry;
    };

    template<>
    class scope<scope_t::single> : public iscope {
    public:
        static int grain; 
        static std::vector<int> permutation;

        static void compact(size_t n); 
        static void scatter(const std::vector<int>& p);
        scope(int value = 0);
        void eval();
        void shift();
        void shift_back(); 
        scope& operator++ ();
        scope& operator-- ();
        operator size_t () const ;
        bool operator < (size_t lim);
       ~scope();
        virtual bool tunable() const ;
        friend std::ostream& operator<< (std::ostream& os, scope const& l){
            os << static_cast<size_t>(l);
            return os;
        }
        std::vector<int> map;
        size_t index;
        bool dry;
        int iterator;
        int factor;
        int round;
    };

    #ifdef AMBIENT_BUILD_LIBRARY
    int scope<scope_t::single>::grain = 1;
    std::vector<int> scope<scope_t::single>::permutation;
    #endif

    template<>
    class scope<scope_t::dedicated> : public iscope {
    public:
        scope();
       ~scope();
        virtual bool tunable() const ;
    };

    template<>
    class scope<scope_t::shared> : public iscope {
    public:
        scope();
       ~scope();
        virtual bool tunable() const ;
    };
}

#endif
