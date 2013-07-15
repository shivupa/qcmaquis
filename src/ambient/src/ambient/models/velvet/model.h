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

#ifndef AMBIENT_MODELS_VELVET_MODEL
#define AMBIENT_MODELS_VELVET_MODEL
#include "ambient/utils/dim2.h"

#include "ambient/models/velvet/revision.h"
#include "ambient/models/velvet/history.h"
#include "ambient/models/velvet/transformable.h"

namespace ambient { namespace models { namespace velvet {

    class model : public singleton< model > {
    public:
        model() : clock(1), sid(0), op_sid(0) {}
        template<ambient::locality L, typename G> 
        void add_revision(history* o, G g);
        void use_revision(history* o);
        bool feeds(const revision* r);
        bool common(const revision* r);
        size_t time(const history* o);
        void touch(const history* o);
        void index(revision* r);
        void index(const transformable* v);
        size_t clock;
        size_t op_sid;
    private:
        int sid;
    };

} } }

namespace ambient {
    extern models::velvet::model& model;
}

#include "ambient/models/velvet/model.hpp"
#include "ambient/models/velvet/transformable.hpp"
#include "ambient/models/velvet/history.hpp"
#include "ambient/models/velvet/revision.hpp"
#endif
