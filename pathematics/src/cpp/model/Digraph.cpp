//
// $Id: Digraph.cpp,v 1.7 2004/08/11 23:30:54 clamatius Exp $

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

#include "model/Digraph.h"

namespace wb_pathematics
{

vector<Edge> blankEdges;

void Digraph::addEdge(const Node& start, const Node& end, tEdgeCost cost)
{
    PM_ASSERT(start.getId() >= 0, "node id >= 0");
    PM_ASSERT(start.getId() < _nodes.size(), "node id < edges.size");
    PM_ASSERT(end.getId() >= 0, "node id >= 0");
    PM_ASSERT(end.getId() < _nodes.size(), "node id < edges.size");

    if (nodeIdValid(start.getId()) && nodeIdValid(end.getId())) {
        addEdge(start.getId(), end.getId(), cost);
    }
}

void Digraph::addEdge(tNodeId startId, tNodeId endId, tEdgeCost cost)
{
    PM_ASSERT(startId >= 0, "node id >= 0");
    PM_ASSERT(startId < _nodes.size(), "node id < edges.size");
    PM_ASSERT(endId >= 0, "node id >= 0");
    PM_ASSERT(endId < _nodes.size(), "node id < edges.size");

    if (nodeIdValid(startId) && nodeIdValid(endId)) {
        NodeInfo& info = _nodes[startId];
        Edge edge(startId, endId, cost);
        info.edges.push_back(edge);
    }
}

void Digraph::addNode(const Node& node)
{
    PM_ASSERT(node.getId() >= 0, "node id >= 0");

    if (node.getId() >= 0) {
        expandNodes(node);
        
        NodeInfo info;
        info.node = node;
        
        _nodes[node.getId()] = info;
    }
}


vector<Edge>& Digraph::getEdges(tNodeId forNodeId)
{
    PM_ASSERT(forNodeId >= 0, "node id >= 0");
    PM_ASSERT(forNodeId < _nodes.size(), "node id < edges.size");

    if (nodeIdValid(forNodeId)) {
        return _nodes[forNodeId].edges;

    } else {
        return blankEdges;
    }
}


Node& Digraph::getNode(int forNodeId)
{
    PM_ASSERT(forNodeId >= 0, "node id >= 0");
    PM_ASSERT(forNodeId < _nodes.size(), "node id < edges.size");

    if (nodeIdValid(forNodeId)) {
        return _nodes[forNodeId].node;

    } else {
        return _nodes[0].node;
    }
}


int Digraph::getNumNodes() const
{
    return _nodes.size();
}


void Digraph::expandNodes(const Node& node)
{
    int id = node.getId();
    if (id >= 0) {
        if (id >= _nodes.size()) {
            _nodes.resize(id+1);
        }
    }
}


void Digraph::clear()
{
    _nodes.clear();
}


void Digraph::resize(int size)
{
    _nodes.resize(size);
}


void Digraph::deleteEdge(tNodeId startId, tNodeId endId)
{
    PM_ASSERT(startId >= 0, "node id >= 0");
    PM_ASSERT(startId < _nodes.size(), "node id < edges.size");
    PM_ASSERT(endId >= 0, "end node id >= 0");
    PM_ASSERT(endId < _nodes.size(), "end node id < edges.size");

    if (nodeIdValid(startId) && nodeIdValid(endId)) {
        vector<Edge>& edges = getEdges(startId);
        for (vector<Edge>::iterator ii = edges.begin(); ii != edges.end(); ii++) {
            if (ii->getEndId() == endId) {
                edges.erase(ii);
                break;
            }
        }
    }
}


bool Digraph::nodeIdValid(tNodeId id)
{
    return (id >= 0 && id < _nodes.size());
}


Edge* Digraph::getEdge(tNodeId startId, tNodeId endId)
{
    vector<Edge>& edges = getEdges(startId);
    for (vector<Edge>::iterator edge = edges.begin(); edge != edges.end(); edge++) {
        if (edge->getEndId() == endId) {
            return &(*edge);
        }
    }
    return NULL;
}


}
