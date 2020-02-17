//
// $Id: Digraph.h,v 1.13 2003/04/30 02:21:24 clamatius Exp $

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

#ifndef __PATHEMATICS_MODEL_DIGRAPH
#define __PATHEMATICS_MODEL_DIGRAPH

#include "PathematicsTypedefs.h"
#include "model/Edge.h"
#include "model/Node.h"

namespace wb_pathematics
{

class Digraph
{
public:

    void addNode(const Node& node);

    void addEdge(tNodeId startId, tNodeId endId, tEdgeCost cost);
    
    void addEdge(const Node& start, const Node& end, tEdgeCost cost);

    void deleteEdge(tNodeId startId, tNodeId endId);

    Edge* getEdge(tNodeId startId, tNodeId endId);

    Node& getNode(tNodeId forNodeId);

    vector<Edge>& getEdges(tNodeId forNodeId);

    int getNumNodes() const;

    void clear();

    void resize(int size);

    bool nodeIdValid(tNodeId id);

protected:

    class NodeInfo
    {
    public:
        Node node;
        vector<Edge> edges;

        NodeInfo()
        {
        }

        NodeInfo(const NodeInfo& other) :
            node(other.node),
            edges(other.edges)
        {
        }
    };

    void expandNodes(const Node& node);

    vector<NodeInfo> _nodes;
};


}

#endif // __PATHEMATICS_MODEL_DIGRAPH
