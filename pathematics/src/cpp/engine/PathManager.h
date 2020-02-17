//
// $Id: PathManager.h,v 1.17 2005/02/19 07:39:47 clamatius Exp $

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

#ifndef __PATHEMATICS_ENGINE_PATHMANAGER
#define __PATHEMATICS_ENGINE_PATHMANAGER

#include "PathematicsTypedefs.h"
#include "engine/PathData.h"
#include "engine/NodeIdBag.h"
#include "model/Edge.h"
#include "model/Node.h"
#include "model/TerrainData.h"
#include "engine/Reward.h"

namespace wb_pathematics
{

typedef Node tTerrainNode;
typedef vector<Edge> tTerrainEdgeVector;
typedef Digraph tTerrainGraph;

// return true if this path is valid for propagating rewards
typedef bool (*tPathValidatorFn)(void* pRock, int upstreamNodeId, int thisNodeId);

class PathManager
{
public:

    PathManager(float rewardFactor);

    void setTerrain(tTerrainGraph* terrain);

    void setRootNode(tNodeId rootNodeId);

    void rebuildTree();

    void reoptimiseTree(int iterations);

    void addCorrectionToApply(tEdgeCost newCost);

    void applyCorrections(tEdgeCost edgeCosts);

    void clearRewardsAt(tNodeId atNodeId);

    void trickleRewards(const vector<Reward>& rewards);

    tDistanceEstimate getDistance(tNodeId forNodeId);

    tNodeId getOptimalNextNodeId();

    tNodeId optimiseNode(tNodeId currentNodeId);

    inline tNodeId getRootNodeId() { return _rootNodeId; }

    inline tTerrainGraph* getTerrain() { return _terrain; }

    inline bool nodeIdValid(tNodeId nodeId) { return (_terrain != NULL) ? _terrain->nodeIdValid(nodeId) : false; }

	inline bool treeValid() { return _treeValid; }

    inline void resetLastNodeId() { _lastRootNodeId = INVALID_NODE_ID; }

	inline void setPathValidatorFn(tPathValidatorFn newFn, void* pRock) { _pathValidator = newFn; _validatorRock = pRock; }

protected:

    void addNodes(tNodeId forNode, bit_vector& nodesUsed);

    void trickleRewards(tNodeId forNodeId);

    tNodeId getRandomNodeId();

    float _rewardFactor;

    float _correctionsToApply; 

    // We model our search spanning tree as a vector, where the edges in the tree are represented
    // by edges in the terrain digraph.
    vector<PathData> _tree;

    tNodeId _currentNodeId;

    tNodeId _rootNodeId;

    tNodeId _lastRootNodeId;

    Digraph* _terrain;

    NodeIdBag _interestingNodes;

    bool _treeValid;

	tPathValidatorFn _pathValidator;

	void* _validatorRock;
};

}

#endif // __PATHEMATICS_ENGINE_PATHMANAGER
