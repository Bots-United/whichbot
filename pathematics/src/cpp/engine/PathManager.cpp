//
// $Id: PathManager.cpp,v 1.29 2005/02/19 21:57:09 clamatius Exp $

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
#include "engine/PathManager.h"
#include "engine/PathData.h"
#include "model/TerrainData.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>

namespace wb_pathematics
{

PathManager::PathManager(float rewardFactor) :
    _terrain(NULL),
    _tree(),
    _rootNodeId(INVALID_NODE_ID),
    _lastRootNodeId(INVALID_NODE_ID),
    _currentNodeId(INVALID_NODE_ID),
    _rewardFactor(rewardFactor),
    _treeValid(false),
    _correctionsToApply(0),
	_pathValidator(NULL)
{
}


void PathManager::setRootNode(tNodeId newRootNodeId)
{
    assert(newRootNodeId >= 0);

    if (_terrain != NULL && _terrain->nodeIdValid(newRootNodeId)) {
        tEdgeCost retraceCost = 0;
        bool mustRebuildTree = (_tree.size() == 0) || (_tree.size() < _terrain->getNumNodes());
        if (!mustRebuildTree) {
			if (!_pathValidator || _pathValidator(_validatorRock, _rootNodeId, newRootNodeId)) {
	            retraceCost = _tree[newRootNodeId].getDistance();
			} else {
				// If we just went through a one-way path, then we should recalculate all the distance estimates
				retraceCost = _tree[newRootNodeId].getDistance() + MAX_EDGE_COST;
			}
        }
        
        if (_rootNodeId >= 0) {
            clearRewardsAt(_rootNodeId);
        }
        
        _lastRootNodeId = _rootNodeId;
        _rootNodeId = newRootNodeId;
        _currentNodeId = _rootNodeId;
        
        if (mustRebuildTree) {
            rebuildTree();
            
        } else {
            _currentNodeId = _rootNodeId;
            // we apply the root-change heuristic whereby we assume we can get to all nodes via going back to the
            // root and using the same distance estimates
            applyCorrections(retraceCost);
            _tree[_rootNodeId].setDistance(0);
            clearRewardsAt(_rootNodeId);
        }
        
        // the tree won't work if we can't go anywhere
        _treeValid = (_terrain->getEdges(_rootNodeId).size() > 0);
    }
}


void PathManager::rebuildTree()
{
    _correctionsToApply = 0;
    _tree.clear();
    
    int numNodes = _terrain->getNumNodes();
    bit_vector nodesUsed(numNodes);

    _tree.resize(numNodes);
    _interestingNodes.setMaxSize(numNodes);

    _tree[_rootNodeId] = PathData(0);
    _currentNodeId = _rootNodeId;

    _treeValid = nodeIdValid(_rootNodeId) && (_terrain->getEdges(_rootNodeId).size() > 0);
}


tNodeId PathManager::getRandomNodeId()
{
    // TODO - we should probably check to make sure this is a valid node id
    // in case the node ids are not contiguous

    assert(_interestingNodes.getNumNodesSet() > 0);

    return _interestingNodes.removeRandomNodeId();
}


void PathManager::reoptimiseTree(int iterations)
{
    if (!_treeValid) {
        return;
    }

    int count = 0;

    if (_currentNodeId != _rootNodeId) {
        optimiseNode(_rootNodeId);
    }

    while (count < iterations) {
        count++;
        
        //printf("Currently optimising node %d\n", _currentNodeId);
        
        tNodeId nextNodeId = optimiseNode(_currentNodeId);

        if (nextNodeId != INVALID_NODE_ID) {
            _currentNodeId = nextNodeId;
            
        } else {
            if (_interestingNodes.getNumNodesSet() > 0) {
                _currentNodeId = getRandomNodeId();

            } else {
                // we have fully optimised the tree, apparently
                break;
            }
            //printf("Choosing random node id %d\n", _currentNodeId);
        }
    }
}


tNodeId PathManager::optimiseNode(tNodeId currentNodeId)
{
	if (!_treeValid) {
		return -1;
	}
    PathData& pathData = _tree[_currentNodeId];
    
    tTerrainEdgeVector& edges = _terrain->getEdges(currentNodeId);
    
    // let's assert that our starting node has some edges
    assert((_currentNodeId != _rootNodeId) || (edges.size() > 0));
    
    tDistanceEstimate largestUpdate = 0;
    tNodeId nextNodeId = INVALID_NODE_ID;
    
    for (tTerrainEdgeVector::iterator ii = edges.begin(); ii != edges.end(); ii++) {
        PM_ASSERT(ii->getEndId() >= 0, "valid endpoint in reoptimiseTree");
        
        tNodeId endpointId = ii->getEndId();
        PathData& endpoint = _tree[endpointId];
        
		bool pathValid = _pathValidator == NULL || _pathValidator(_validatorRock, endpointId, currentNodeId);
		float cost = ii->getCost();
        tDistanceEstimate expectedDistance = pathData.getDistance() + cost;
        tDistanceEstimate actualDistance = endpoint.getDistance();
        
        //printf("Actual distance to %d: %f, expected %f, cost %f\n", 
        //    endpointId, actualDistance, expectedDistance, ii->getCost());
		if (pathValid) {
			if (actualDistance > expectedDistance) {
				tDistanceEstimate correction = actualDistance - expectedDistance;
	            
				if (correction > largestUpdate) {
					correction = largestUpdate;
					nextNodeId = endpointId;
					endpoint.setDistance(expectedDistance);
				}
	            
				_interestingNodes.addNodeId(endpointId);
			}

		} else {
			endpoint.setDistance(MAX_EDGE_COST);
		}
    }

    return nextNodeId;
}


void PathManager::applyCorrections(tDistanceEstimate additionalEstimates)
{
    float totalCorrection = _correctionsToApply + additionalEstimates;

    if (totalCorrection > 0) {
        for (vector<PathData>::iterator ii = _tree.begin(); ii != _tree.end(); ii++) {
            PathData& data = *ii;
            data.setDistance(data.getDistance() + totalCorrection);
        }
    }

    _correctionsToApply = 0;
}


void PathManager::clearRewardsAt(tNodeId atNodeId)
{
    assert(atNodeId >= 0);
    assert(atNodeId < _tree.size());

    if ((_terrain != NULL) && (_terrain->nodeIdValid(atNodeId))) {
        _tree[atNodeId].setReward(0);
        _tree[atNodeId].setCumulativeReward(0);
    }
}


void PathManager::trickleRewards(const vector<Reward>& rewards)
{
    // reset all the rewards
    for (int nodeId = 0; nodeId < _tree.size(); nodeId++) {
		_tree[nodeId].setReward(0);
        _tree[nodeId].setCumulativeReward(0);
    }

    for (vector<Reward>::const_iterator jj = rewards.begin(); jj != rewards.end(); jj++) {
        _tree[jj->getNodeId()].addReward(jj->getValue());
    }

    for (vector<Reward>::const_iterator ii = rewards.begin(); ii != rewards.end(); ii++) {
        trickleRewards(ii->getNodeId());
    }
}


void PathManager::trickleRewards(tNodeId forNodeId)
{
    assert(forNodeId >= 0);
    assert(forNodeId < _tree.size());

    if (!_treeValid) {
        return;
    }

    if (_terrain != NULL && _terrain->nodeIdValid(forNodeId)) {
        tNodeId currentNodeId = forNodeId;
        tDistanceEstimate ourDistance = 0;
        tNodeId nextNodeId = 0;
        int count = 0;
        tReward cumulativeReward = 0;
        
        do {
            count++;
            
            ourDistance = _tree[currentNodeId].getDistance();
            if (ourDistance >= MAX_DISTANCE_ESTIMATE) {
                // this node doesn't have a real distance estimate yet, let's just return and save some cycles
                return;
            }
            
            if (ourDistance == 0) {
                // we're at the root
                return;
            }
            
            cumulativeReward = _tree[currentNodeId].getReward();
            
            tTerrainEdgeVector& edges = _terrain->getEdges(currentNodeId);
            
            nextNodeId = INVALID_NODE_ID;
            tDistanceEstimate lowestDistance = MAX_DISTANCE_ESTIMATE*2;
            
            // calculate our new cumulative reward from all the upstream nodes
            for (tTerrainEdgeVector::iterator ii = edges.begin(); ii < edges.end(); ii++) {
                tNodeId endpointId = ii->getEndId();
                PathData& data = _tree[endpointId];
				tDistanceEstimate dataDistance = data.getDistance();
                if (data.getDistance() >= ourDistance) {
	                cumulativeReward += data.getCumulativeReward();
                    
                } else {
                    if (data.getDistance() < lowestDistance) {
                        lowestDistance = data.getDistance();
						// if this node appears to be the closest to the root, let's go that way next
                        nextNodeId = endpointId;
                    }
                }
            }
            
            _tree[currentNodeId].setCumulativeReward(cumulativeReward);
            
            currentNodeId = nextNodeId;
            
        } while ((currentNodeId != INVALID_NODE_ID) && count <= _tree.size());
    }
} 


tNodeId PathManager::getOptimalNextNodeId()
{
    tTerrainEdgeVector& edges = _terrain->getEdges(_rootNodeId);
    
    tReward highestReward = -1000000;
    tNodeId nextNodeId = INVALID_NODE_ID;

    for (tTerrainEdgeVector::iterator ii = edges.begin(); ii < edges.end(); ii++) {
        tNodeId nodeId = ii->getEndId();
        
        PathData& data = _tree[nodeId];
        
        if (data.getCumulativeReward() >= highestReward) {
            // momentum check to avoid getting stuck in local cycles
            if ((edges.size() == 1) || (nodeId != _lastRootNodeId))
            {
                highestReward = data.getCumulativeReward();
                nextNodeId = nodeId;
            }
        }
    }
    
    return nextNodeId;
}


tDistanceEstimate PathManager::getDistance(tNodeId forNodeId)
{
    assert(forNodeId >= 0);
    assert(forNodeId < _tree.size());

    if ((_terrain != NULL) && (_terrain->nodeIdValid(forNodeId))) {
        return _tree[forNodeId].getDistance();

    } else {
        return MAX_DISTANCE_ESTIMATE;
    }
}


void PathManager::setTerrain(tTerrainGraph* terrain)
{
    _terrain = terrain;
    _currentNodeId = INVALID_NODE_ID;
    _rootNodeId = INVALID_NODE_ID;
    _tree.clear();
    _treeValid = false;
}


void PathManager::addCorrectionToApply(tEdgeCost newCost)
{
    assert(newCost > 0);
    if (newCost > 0) {
        _correctionsToApply += newCost;
    }
}


}
