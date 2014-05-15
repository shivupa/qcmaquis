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

#ifndef AMBIENT
#define AMBIENT
// {{{ system includes
#ifndef AMBIENT_NOP_CHANNEL
#include <mpi.h>
#endif
#include <complex>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <limits>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <memory.h>
#include <stdarg.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <algorithm>
#include <execinfo.h>
#include <stdexcept>
// }}}

#define AMBIENT_IB                    2048
#define AMBIENT_INSTR_BULK_CHUNK      16777216 // 16 MB
#define AMBIENT_DATA_BULK_CHUNK       67108864 // 64 MB
#define AMBIENT_MAX_SID               2097152  // Cray MPI
#define AMBIENT_MPI_THREADING         MPI_THREAD_FUNNELED
#define AMBIENT_MASTER_RANK           0

#include "ambient/utils/dim2.h"
#include "ambient/utils/enums.h"
#include "ambient/utils/tree.hpp"
#include "ambient/utils/fence.hpp"
#include "ambient/utils/enable_threading.hpp"
#include "ambient/utils/math.hpp"
#include "ambient/utils/rank_t.hpp"

#include "ambient/memory/pool.hpp"
#include "ambient/memory/new.h"
#include "ambient/memory/allocator.h"

#include "ambient/models/ssm/revision.h"
#include "ambient/models/ssm/history.h"
#include "ambient/models/ssm/transformable.h"
#include "ambient/models/ssm/model.h"

#ifndef AMBIENT_NOP_CHANNEL
#include "ambient/channels/mpi/group.h"
#include "ambient/channels/mpi/multirank.h"
#include "ambient/channels/mpi/channel.h"
#include "ambient/channels/mpi/request.h"
#include "ambient/channels/mpi/collective.h"
#else
#include "ambient/channels/nop/channel.h"
#endif

#include "ambient/controllers/ssm/functor.h"
#include "ambient/controllers/ssm/collector.h"
#include "ambient/controllers/ssm/controller.h"
#include "ambient/controllers/ssm/get.h"
#include "ambient/controllers/ssm/set.h"
#include "ambient/controllers/ssm/scope.h"
#include "ambient/controllers/ssm/workflow.h"

#include "ambient/utils/auxiliary.hpp"
#include "ambient/utils/io.hpp"

#include "ambient/memory/new.hpp"
#include "ambient/memory/allocator.hpp"
#include "ambient/memory/data_bulk.hpp"
#include "ambient/memory/instr_bulk.hpp"

#include "ambient/models/ssm/revision.hpp"
#include "ambient/models/ssm/history.hpp"
#include "ambient/models/ssm/transformable.hpp"
#include "ambient/models/ssm/model.hpp"

#ifndef AMBIENT_NOP_CHANNEL
#include "ambient/channels/mpi/group.hpp"
#include "ambient/channels/mpi/multirank.hpp"
#include "ambient/channels/mpi/channel.hpp"
#include "ambient/channels/mpi/request.hpp"
#include "ambient/channels/mpi/collective.hpp"
#endif

#include "ambient/controllers/ssm/get.hpp"
#include "ambient/controllers/ssm/set.hpp"
#include "ambient/controllers/ssm/collector.hpp"
#include "ambient/controllers/ssm/controller.hpp"
#include "ambient/controllers/ssm/scope.hpp"
#include "ambient/controllers/ssm/workflow.hpp"

#include "ambient/interface/typed.hpp"
#include "ambient/interface/kernel.hpp"
#include "ambient/interface/access.hpp"
#include "ambient/interface/lambda.hpp"
#include "ambient/interface/algorithms.hpp"

#include "ambient/container/proxy.hpp"
#include "ambient/container/block.hpp"
#endif
