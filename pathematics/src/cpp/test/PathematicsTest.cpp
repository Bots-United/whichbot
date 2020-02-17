//
// $Id: PathematicsTest.cpp,v 1.17 2005/02/22 06:03:19 clamatius Exp $

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

#include "model/TerrainData.h"
#include "engine/PathManager.h"

using namespace wb_pathematics;

#include <stdio.h>

static int id = 0;
void addNode(tTerrainGraph& graph, float x, float y, float z)
{
    tTerrainNode node(id, TerrainData(Vector3D(x, y, z)));
    graph.addNode(node);
    id++;
}

auto_ptr<tTerrainGraph> createGraph()
{
    tTerrainGraph* pGraph = new tTerrainGraph();

    addNode(*pGraph, 0, 0, 0); // node 0
    addNode(*pGraph, 1, 0, 0); // node 1
    addNode(*pGraph, 0, 1, 0); // node 2

    addNode(*pGraph, 2, 0, 0); // node 3
    addNode(*pGraph, 1, 1, 0); // node 4
    addNode(*pGraph, 0, 2, 0); // node 5

    pGraph->addEdge(0, 1, 1.0);
    pGraph->addEdge(1, 0, 1.0);

    pGraph->addEdge(0, 2, 1.0);
    pGraph->addEdge(2, 0, 1.0);
    pGraph->addEdge(1, 2, 1.4142136f);
    pGraph->addEdge(2, 1, 1.4142136f);

    pGraph->addEdge(1, 3, 1.0);
    pGraph->addEdge(3, 1, 1.0);
    pGraph->addEdge(3, 4, 1.4142136f);
    pGraph->addEdge(4, 3, 1.4142136f);
    pGraph->addEdge(1, 4, 1.0);
    pGraph->addEdge(4, 1, 1.0);
    pGraph->addEdge(0, 4, 1.4142136f);
    pGraph->addEdge(4, 0, 1.0);
    pGraph->addEdge(4, 5, 1.0);
    pGraph->addEdge(5, 4, 1.0);
    pGraph->addEdge(2, 5, 1.0);
    pGraph->addEdge(5, 2, 1.0);

    return auto_ptr<tTerrainGraph>(pGraph);
}

tDistanceEstimate getDistance(PathManager& pathMgr, tNodeId nodeId)
{
    return pathMgr.getDistance(nodeId);
}

void runBasicTest()
{
    auto_ptr<tTerrainGraph> terrain(createGraph());

    PathManager pathMgr(1.0);
    pathMgr.setTerrain(terrain.get());
    pathMgr.setRootNode(0);
    printf("About to build tree...\n");
    pathMgr.rebuildTree();
    printf("Path rebuilt.\n");

    pathMgr.reoptimiseTree(10);

    for (int ii = 0; ii < 6; ii++) {
        printf("Distance to node %d is %f\n", ii, getDistance(pathMgr, ii));
    }
    PM_ASSERT(getDistance(pathMgr, 0) == 0, "root node is at 0");
    PM_ASSERT(getDistance(pathMgr, 1) == 1, "node 1 is at 1");
    PM_ASSERT(getDistance(pathMgr, 2) == 1, "node 2 is at 1");
    PM_ASSERT(getDistance(pathMgr, 3) == 2, "node 3 is at 2");
    PM_ASSERT(getDistance(pathMgr, 5) == 2, "node 5 is at 2");

    pathMgr.setRootNode(1);

    printf("Distance to root node: %f\n", getDistance(pathMgr, 1));
    PM_ASSERT(getDistance(pathMgr, 1) == 0, "root node is at 0");
    PM_ASSERT(getDistance(pathMgr, 0) == 1, "node 0 is at 1");
    PM_ASSERT(getDistance(pathMgr, 3) == 3, "node 3 is at 3");

    pathMgr.reoptimiseTree(10);

    PM_ASSERT(getDistance(pathMgr, 3) == 1, "node 3 is at 1");

    pathMgr.setRootNode(0);

    pathMgr.reoptimiseTree(10);

    vector<Reward> rewards;
    rewards.push_back(Reward(5, 1.0, "5"));
    rewards.push_back(Reward(4, 0.5, "4"));

    pathMgr.trickleRewards(rewards);

    printf("Optimal next node: %d\n", pathMgr.getOptimalNextNodeId());

    pathMgr.setRootNode(4);
    pathMgr.trickleRewards(rewards);
    printf("Optimal next node: %d\n", pathMgr.getOptimalNextNodeId());

}

void buildNodeGrid(tTerrainGraph& terrain, int size)
{
    for (int xx = 0; xx < size; xx++) {
        for (int yy = 0; yy < size; yy++) {
            terrain.addNode(Node(yy * size + xx, TerrainData(Vector3D((float)xx, (float)yy, 0.0))));
        }
    }
}


void addEdgesToGrid(tTerrainGraph& terrain, int size)
{
    for (int xx = 0; xx < size; xx++) {
        for (int yy = 0; yy < size; yy++) {
            int id = yy * size + xx;
            if (xx > 0) {
                terrain.addEdge(id, id - 1, 1.0f);
            }
            if (xx < size-1) {
                terrain.addEdge(id, id + 1, 1.0f);
            }

            if (yy > 0) {
                terrain.addEdge(id, id - size, 1.0f);
            }

            if (yy < size-1) {
                terrain.addEdge(id, id + size, 1.0f);
            }
        }
    }
}

void printDistances(PathManager& pathMgr, int size)
{
    for (int xx = 0; xx < size; xx++) {
        for (int yy = 0; yy < size; yy++) {
            int id = yy * size + xx;

            tDistanceEstimate distance = pathMgr.getDistance(id);


            if (distance >= MAX_DISTANCE_ESTIMATE) {
                printf("%s ", "X");

            } else {
                int printableDistance = (int)((distance * 10000.0)/100);
                printf("%d ", printableDistance);
            }
        }

        printf("\n");
    }
}

void runConvergenceTest()
{
    int size = 10;
    tTerrainGraph terrain;
    buildNodeGrid(terrain, size);
    addEdgesToGrid(terrain, size);


    PathManager pathMgr(1.0);
    pathMgr.setTerrain(&terrain);

    pathMgr.setRootNode(0);

    pathMgr.reoptimiseTree(15);

    printDistances(pathMgr, size);
}


bool pathValidatorFn(void* rock, int upstreamNodeId, int thisNodeId)
{
	// The one-way path here is from node 1 to node 2 - so when node 1 is the upstream
	// node, going that way is not allowed
	if (thisNodeId == 2) {
		if (upstreamNodeId == 1) {
			return false;
		}
	}
	return true;
}


void runOneWayPathTest()
{
    tTerrainGraph* pGraph = new tTerrainGraph();

    addNode(*pGraph, 0, 0, 0); // node 0
    addNode(*pGraph, 1, 0, 0); // node 1
    addNode(*pGraph, 0, 1, 0); // node 2

    addNode(*pGraph, 2, 0, 0); // node 3
    addNode(*pGraph, 1, 1, 0); // node 4
    addNode(*pGraph, 0, 2, 0); // node 5

	// Set up a ring
    pGraph->addEdge(0, 1, 0.5);
    pGraph->addEdge(1, 0, 0.5);
	pGraph->addEdge(1, 2, 1.0);
	pGraph->addEdge(2, 1, 1.0);
	pGraph->addEdge(2, 3, 1.0);
	pGraph->addEdge(3, 2, 1.0);
	pGraph->addEdge(3, 4, 1.0);
	pGraph->addEdge(4, 3, 1.0);
	pGraph->addEdge(4, 5, 1.0);
	pGraph->addEdge(5, 4, 1.0);
 	pGraph->addEdge(5, 0, 1.0);
	pGraph->addEdge(0, 5, 1.0);

	PathManager pathMgr(1.0);
	pathMgr.setTerrain(pGraph);
	pathMgr.setPathValidatorFn(pathValidatorFn, NULL);
	pathMgr.setRootNode(0);
	pathMgr.reoptimiseTree(15);

	vector<Reward> rewards;
	rewards.push_back(Reward(0, 0.5, "0"));
    rewards.push_back(Reward(3, 1.0, "3"));

    pathMgr.trickleRewards(rewards);
	printf("Optimal next node: %d\n", pathMgr.getOptimalNextNodeId());
	// This should be 1, as that is the closest path
	assert(pathMgr.getOptimalNextNodeId() == 1);

	pathMgr.setRootNode(3);
	pathMgr.reoptimiseTree(15);
	printf("Optimal next node: %d\n", pathMgr.getOptimalNextNodeId());

	// This should be 4, as that is the only path back to 0 (1->2 is one-way)
	assert(pathMgr.getOptimalNextNodeId() == 4);
}


int main(char** argv, int argc)
{

    //runBasicTest();

    //runConvergenceTest();

	runOneWayPathTest();

    printf("Test passed!\n");
    return 0;
}
