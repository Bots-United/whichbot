//
// $Id: NodeIdBag.cpp,v 1.3 2004/08/11 22:53:23 clamatius Exp $

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

#include "PathematicsTypedefs.h"
#include "engine/NodeIdBag.h"
#include "model/Node.h"

namespace wb_pathematics
{

NodeIdBag::NodeIdBag()
{
    clear();
}

void NodeIdBag::clear()
{
    _nodes.clear();
    _lowestNodeId = INVALID_NODE_ID;
    _highestNodeId = INVALID_NODE_ID;
    _numNodesSet = 0;
}

void NodeIdBag::setMaxSize(int newSize)
{
    _nodes.resize(newSize);
}

tNodeId NodeIdBag::removeRandomNodeId()
{
    if (_numNodesSet == 0) {
        return INVALID_NODE_ID;
    }

    int returnVal = INVALID_NODE_ID;

    if (_numNodesSet == 1) {
        returnVal = _lowestNodeId;

    } else {

        int maxVal = _highestNodeId - _lowestNodeId;
        int randomId = _lowestNodeId + (int)((1.0 * (maxVal-1) * rand()) / RAND_MAX);

        // if that node id isn't in the bag, keep looking upwards till we find a node id
        // that is in the bag.  skip over if we get to the max value (highestnodeid).
        while (!_nodes[randomId]) {
            randomId++;
            if (randomId > _highestNodeId) {
                randomId = _lowestNodeId;
            }
        }

        returnVal = randomId;
    }

    _numNodesSet--;
    return returnVal;
}

void NodeIdBag::addNodeId(tNodeId nodeId)
{
    if (_numNodesSet == 0) {
        _highestNodeId = nodeId;
        _lowestNodeId = nodeId;

    } else if (nodeId > _highestNodeId) {
        _highestNodeId = nodeId;

    } else if (nodeId < _lowestNodeId) {
        _lowestNodeId = nodeId;
    }

    _nodes[nodeId] = true;
    _numNodesSet++;
}

int NodeIdBag::getNumNodesSet() const
{
    return _numNodesSet;
}

}
