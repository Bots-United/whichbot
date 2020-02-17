//
// $Id: NodeIdBag.h,v 1.2 2003/04/10 18:14:52 clamatius Exp $

// Copyright (c) 2003, WhichBot Project
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the WhichBot Project nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Original Pathematics algorithm by Alex Champandard (http://www.base-sixteen.com/)
// This implementation by Mike Cooper.

#ifndef __PATHEMATICS_ENGINE_NODEIDBAG
#define __PATHEMATICS_ENGINE_NODEIDBAG

#include "PathematicsTypedefs.h"

namespace wb_pathematics
{

class NodeIdBag
{
public:

    NodeIdBag();

    void setMaxSize(int newSize);

    tNodeId removeRandomNodeId();

    void addNodeId(tNodeId id);

    int getNumNodesSet() const;

    void clear();

protected:

    int _lowestNodeId;
    int _highestNodeId;
    int _numNodesSet;
    bit_vector _nodes;
};

}

#endif // __PATHEMATICS_ENGINE_NODEIDBAG
