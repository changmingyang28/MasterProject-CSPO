/** @file PseudoDet.cpp 
 * 
 * Implementation of pseudo-determinization algorithm for Rabin automata
 */

 #include "libfaudes.h"

 namespace faudes {
 
 /*
 ********************************
 TreeNode Implementation
 ********************************
 */
 
 TreeNode::TreeNode() : color(WHITE) {}
 
 bool TreeNode::operator<(const TreeNode& other) const {
     if(stateLabel != other.stateLabel) return stateLabel < other.stateLabel;
     if(aSet != other.aSet) return aSet < other.aSet;
     if(rSet != other.rSet) return rSet < other.rSet;
     if(children != other.children) return children < other.children;
     return color < other.color;
 }
 
 bool TreeNode::operator==(const TreeNode& other) const {
     return stateLabel == other.stateLabel && 
            aSet == other.aSet && 
            rSet == other.rSet && 
            children == other.children && 
            color == other.color;
 }
 
 std::string TreeNode::ToString() const {
     std::ostringstream oss;
     oss << "Node{states=[" << stateLabel.ToString() << "], ";
     oss << "color=" << (color == WHITE ? "WHITE" : (color == RED ? "RED" : "GREEN")) << ", ";
     oss << "children=" << children.size() << ", ";
     oss << "aSet=" << aSet.size() << ", ";
     oss << "rSet=" << rSet.size() << "}";
     return oss.str();
 }
 
 /*
 ********************************
 LabeledTree Implementation
 ********************************
 */
 
 LabeledTree::LabeledTree() : rootNode(INVALID_NODE), nextNodeId(1) {}
 
 Idx LabeledTree::createNode() {
     Idx id = nextNodeId++;
     nodes[id] = TreeNode();
     return id;
 }
 
 void LabeledTree::deleteNode(Idx nodeId) {
     if(nodes.find(nodeId) == nodes.end()) return;
     
     // Remove this node from all A-sets and R-sets
     for(auto& pair : nodes) {
         pair.second.aSet.erase(nodeId);
         pair.second.rSet.erase(nodeId);
         
         // Remove from children
         auto& children = pair.second.children;
         children.erase(std::remove(children.begin(), children.end(), nodeId), children.end());
     }
     
     // Remove the node itself
     nodes.erase(nodeId);
 }
 
 std::string LabeledTree::ToString() const {
     std::ostringstream oss;
     oss << "Tree{root=" << rootNode << ", nodes=[";
     for(auto& pair : nodes) {
         oss << pair.first << ":" << pair.second.ToString() << ", ";
     }
     oss << "]}";
     return oss.str();
 }
 
 bool LabeledTree::operator<(const LabeledTree& other) const {
     if(rootNode != other.rootNode) return rootNode < other.rootNode;
     
     // Compare number of nodes
     if(nodes.size() != other.nodes.size()) return nodes.size() < other.nodes.size();
     
     // Compare nodes one by one
     for(auto it1 = nodes.begin(), it2 = other.nodes.begin();
         it1 != nodes.end() && it2 != other.nodes.end(); ++it1, ++it2) {
         if(it1->first != it2->first) return it1->first < it2->first;
         if(!(it1->second == it2->second)) return it1->second < it2->second;
     }
     
     return false;
 }
 
 bool LabeledTree::operator==(const LabeledTree& other) const {
     if(rootNode != other.rootNode) return false;
     if(nodes.size() != other.nodes.size()) return false;
     
     for(auto it1 = nodes.begin(), it2 = other.nodes.begin();
         it1 != nodes.end() && it2 != other.nodes.end(); ++it1, ++it2) {
         if(it1->first != it2->first) return false;
         if(!(it1->second == it2->second)) return false;
     }
     
     return true;
 }
 
 /*
 ********************************
 Helper Functions
 ********************************
 */
 
 std::string ComputeTreeSignature(const LabeledTree& tree) {
     std::ostringstream oss;
     
     // Recursive signature processing
     std::function<void(Idx)> dfs = [&](Idx nodeId) {
         if(tree.nodes.find(nodeId) == tree.nodes.end()) return;
         
         const TreeNode& node = tree.nodes.at(nodeId);
         oss << nodeId << ":";
         oss << node.stateLabel.ToString() << ":";
         oss << static_cast<int>(node.color) << ":";
         
         // Process children
         for(Idx childId : node.children) {
             dfs(childId);
         }
         
         oss << ";";
     };
     
     dfs(tree.rootNode);
     return oss.str();
 }
 
 /*
 ********************************
 Main Algorithm Implementation
 ********************************
 */
 
 RabinAutomaton PseudoDet(const RabinAutomaton& rGen) {
     FD_DF("PseudoDet(" << rGen.Name() << ")");
     
     // Get input Rabin automaton acceptance condition
     RabinAcceptance inputRabinPairs = rGen.RabinAcceptance();
     std::cout << "Debug: Input generator has " << inputRabinPairs.Size() << " RabinPairs" << std::endl;
     
     RabinAutomaton rResGen;
     rResGen.Clear();
     rResGen.Name(CollapsString("PseudoDet(" + rGen.Name() + ")"));
     
     if(rGen.InitStatesEmpty()) {
         std::cout << "Debug: Input generator has no initial states, returning empty result." << std::endl;
         return rResGen;
     }
     
     // Copy alphabet
     rResGen.InjectAlphabet(rGen.Alphabet());
     
     // Safety limits
     const int MAX_STATES = 1000;
     const int MAX_ITERATIONS = 10000;
     const int MAX_TREE_NODES = 100;
     int stateCounter = 0;
     int iterationCounter = 0;
     
     // Store tree for each state
     std::map<Idx, LabeledTree> stateToTree;
     
     // Track tree signatures
     std::map<std::string, Idx> treeSignatureToState;
     
     // Create initial tree
     LabeledTree initialTree;
     Idx root = initialTree.createNode();
     initialTree.rootNode = root;
     
     // Set root node label to contain all initial states
     for(StateSet::Iterator sit = rGen.InitStatesBegin(); sit != rGen.InitStatesEnd(); ++sit) {
         initialTree.nodes[root].stateLabel.Insert(*sit);
     }
     
     // Create initial state in output automaton
     Idx initialState = rResGen.InsInitState();
     stateToTree[initialState] = initialTree;
     
     std::string initialSig = ComputeTreeSignature(initialTree);
     treeSignatureToState[initialSig] = initialState;
     
     std::queue<Idx> stateQueue;
     stateQueue.push(initialState);
     stateCounter++;
     
     // Process all states
     while(!stateQueue.empty() && stateCounter < MAX_STATES && iterationCounter < MAX_ITERATIONS) {
         iterationCounter++;
         
         Idx currentState = stateQueue.front();
         stateQueue.pop();
         
         LabeledTree currentTree = stateToTree[currentState];
         std::cout << "Debug: Processing state " << currentState << " with tree: " 
                   << currentTree.ToString() << std::endl;
         
         // Process each event
         for(EventSet::Iterator evIt = rGen.AlphabetBegin(); evIt != rGen.AlphabetEnd(); ++evIt) {
             Idx event = *evIt;
             std::cout << "Debug: Processing event " << rGen.EventName(event) << std::endl;
             
             // Clone tree
             LabeledTree newTree = currentTree;
             
             // Skip if tree is too large
             if(newTree.nodes.size() > MAX_TREE_NODES) {
                 std::cout << "Warning: Tree size exceeds limit, skipping event." << std::endl;
                 continue;
             }
             
             // Step 1: Color all nodes white
             for(auto& pair : newTree.nodes) {
                 pair.second.color = TreeNode::WHITE;
             }
             
             // Step 2: Update state labels based on transitions
             for(auto& pair : newTree.nodes) {
                 Idx nodeId = pair.first;
                 TreeNode& node = pair.second;
                 
                 // Create new label based on successor states
                 StateSet newLabel;
                 
                 // Check if it's an epsilon event
                 bool isEpsilonEvent = false;
                 std::string eventName = rGen.EventName(event);
                 if(eventName.find("eps") != std::string::npos || eventName == "epsilon") {
                     isEpsilonEvent = true;
                 }
                 
                 if(isEpsilonEvent) {
                     // For epsilon transitions: δ(ε, Y) ∪ Y
                     // First include original state set Y
                     for(StateSet::Iterator sit = node.stateLabel.Begin(); sit != node.stateLabel.End(); ++sit) {
                         newLabel.Insert(*sit);
                     }
                     
                     // Then add states reachable through epsilon transitions δ(ε, Y)
                     for(StateSet::Iterator sit = node.stateLabel.Begin(); sit != node.stateLabel.End(); ++sit) {
                         Idx state = *sit;
                         
                         for(TransSet::Iterator tit = rGen.TransRelBegin(state, event); 
                             tit != rGen.TransRelEnd(state, event); ++tit) {
                             newLabel.Insert(tit->X2);
                         }
                     }
                 } else {
                     // For regular events: δ(σ, Y)
                     // Check all transitions from this node with this event
                     for(StateSet::Iterator sit = node.stateLabel.Begin(); sit != node.stateLabel.End(); ++sit) {
                         Idx state = *sit;
                         
                         for(TransSet::Iterator tit = rGen.TransRelBegin(state, event); 
                             tit != rGen.TransRelEnd(state, event); ++tit) {
                             newLabel.Insert(tit->X2);
                         }
                     }
                 }
                 
                 // Update node's state label
                 node.stateLabel = newLabel;
             }
             
             // Step 3: Create nodes for potential acceptance violations
             if(inputRabinPairs.Size() > 0) {
                 // Hard limit on total child nodes
                 const int MAX_TOTAL_CHILDREN = 40;
                 int totalChildrenCreated = 0;
                 
                 for(auto& pair : newTree.nodes) {
                     Idx nodeId = pair.first;
                     TreeNode& node = pair.second;
                     
                     // Break out of entire loop if too many children created
                     if(totalChildrenCreated >= MAX_TOTAL_CHILDREN) {
                         std::cout << "Warning: Reached maximum total children limit." << std::endl;
                         break;
                     }
                     
                     // Limit per node
                     const int MAX_CHILDREN_PER_NODE = 3;
                     int nodeChildrenCount = 0;
                     
                     // For each Rabin pair
                     RabinAcceptance::CIterator rit;
                     Idx i = 0;
                     for(rit = inputRabinPairs.Begin(); 
                         rit != inputRabinPairs.End() && nodeChildrenCount < MAX_CHILDREN_PER_NODE; 
                         ++rit, ++i) {
                         
                         // Check states outside I set
                         StateSet outsideI;
                         const StateSet& I = rit->ISet();
                         
                         for(StateSet::Iterator sit = node.stateLabel.Begin(); 
                             sit != node.stateLabel.End() && outsideI.Size() < 5; ++sit) {
                             if(!I.Exists(*sit)) {
                                 outsideI.Insert(*sit);
                             }
                         }
                         
                         // Only create child if we have states outside I and haven't exceeded limits
                         if(!outsideI.Empty() && nodeChildrenCount < MAX_CHILDREN_PER_NODE) {
                             // Check if this label is already represented in existing children
                             bool alreadyRepresented = false;
                             for(Idx childId : node.children) {
                                 if(newTree.nodes.find(childId) != newTree.nodes.end()) {
                                     // Consider labels equivalent if there's significant overlap
                                     StateSet intersection = newTree.nodes[childId].stateLabel * outsideI;
                                     if(!intersection.Empty()) {
                                         alreadyRepresented = true;
                                         break;
                                     }
                                 }
                             }
                             
                             if(!alreadyRepresented) {
                                 Idx newChild = newTree.createNode();
                                 newTree.nodes[newChild].stateLabel = outsideI;
                                 newTree.nodes[newChild].color = TreeNode::RED;
                                 node.children.push_back(newChild);
                                 
                                 nodeChildrenCount++;
                                 totalChildrenCreated++;
                             }
                         }
                     }
                 }
             }
             
             // Step 4: Maintain state disjointness among siblings
             for(auto& pair : newTree.nodes) {
                 Idx parentId = pair.first;
                 TreeNode& parent = pair.second;
                 
                 // Children are sorted by age (assuming node IDs increase with creation time)
                 // For each child, remove states that appear in older siblings
                 for(size_t i = 1; i < parent.children.size(); ++i) {
                     Idx youngerId = parent.children[i];
                     
                     if(newTree.nodes.find(youngerId) == newTree.nodes.end()) continue;
                     
                     for(size_t j = 0; j < i; ++j) {
                         Idx olderId = parent.children[j];
                         
                         if(newTree.nodes.find(olderId) == newTree.nodes.end()) continue;
                         
                         // Remove all states of older sibling from younger node
                         for(StateSet::Iterator sit = newTree.nodes[olderId].stateLabel.Begin(); 
                             sit != newTree.nodes[olderId].stateLabel.End(); ++sit) {
                             newTree.nodes[youngerId].stateLabel.Erase(*sit);
                         }
                     }
                 }
             }
             
             // Step 5: Remove all nodes with empty state labels
             std::vector<Idx> nodesToRemove;
             for(auto& pair : newTree.nodes) {
                 if(pair.second.stateLabel.Empty()) {
                     nodesToRemove.push_back(pair.first);
                 }
             }
             
             for(Idx nodeId : nodesToRemove) {
                 newTree.deleteNode(nodeId);
             }
             
             // Step 6: Determine red breakpoints
             for(auto& pair : newTree.nodes) {
                 Idx nodeId = pair.first;
                 TreeNode& node = pair.second;
                 
                 // Compute union of children's state labels
                 StateSet unionOfChildren;
                 for(Idx childId : node.children) {
                     if(newTree.nodes.find(childId) == newTree.nodes.end()) continue;
                     
                     for(StateSet::Iterator sit = newTree.nodes[childId].stateLabel.Begin(); 
                         sit != newTree.nodes[childId].stateLabel.End(); ++sit) {
                         unionOfChildren.Insert(*sit);
                     }
                 }
                 
                 if(node.stateLabel == unionOfChildren && !unionOfChildren.Empty()) {
                     std::cout << "Debug: Red breakpoint at node " << nodeId << std::endl;
                     node.color = TreeNode::RED;
                     
                     // Delete all descendants
                     std::vector<Idx> descendants;
                     std::queue<Idx> bfs;
                     for(Idx child : node.children) {
                         if(newTree.nodes.find(child) != newTree.nodes.end()) {
                             bfs.push(child);
                         }
                     }
                     
                     while(!bfs.empty()) {
                         Idx current = bfs.front();
                         bfs.pop();
                         descendants.push_back(current);
                         
                         if(newTree.nodes.find(current) == newTree.nodes.end()) continue;
                         
                         for(Idx child : newTree.nodes[current].children) {
                             if(newTree.nodes.find(child) != newTree.nodes.end()) {
                                 bfs.push(child);
                             }
                         }
                     }
                     
                     for(Idx descendant : descendants) {
                         newTree.deleteNode(descendant);
                     }
                     
                     node.children.clear();
                     node.aSet.clear();
                     node.rSet.clear();
                 }
             }
             
             // Step 7: Update A-sets and R-sets
             // Remove nodes that were deleted in steps 5 and 6
             for(auto& pair : newTree.nodes) {
                 auto& aSet = pair.second.aSet;
                 auto& rSet = pair.second.rSet;
                 
                 std::vector<Idx> toRemoveFromA, toRemoveFromR;
                 
                 for(Idx nodeInA : aSet) {
                     if(newTree.nodes.find(nodeInA) == newTree.nodes.end()) {
                         toRemoveFromA.push_back(nodeInA);
                     }
                 }
                 
                 for(Idx nodeInR : rSet) {
                     if(newTree.nodes.find(nodeInR) == newTree.nodes.end()) {
                         toRemoveFromR.push_back(nodeInR);
                     }
                 }
                 
                 for(Idx nodeId : toRemoveFromA) aSet.erase(nodeId);
                 for(Idx nodeId : toRemoveFromR) rSet.erase(nodeId);
             }
             
             // Step 8: Handle green coloring
             for(auto& pair : newTree.nodes) {
                 Idx nodeId = pair.first;
                 TreeNode& node = pair.second;
                 
                 if(node.aSet.empty() && node.color != TreeNode::RED) {
                     std::cout << "Debug: Green coloring for node " << nodeId << std::endl;
                     node.color = TreeNode::GREEN;
                     node.aSet = node.rSet;
                     node.rSet.clear();
                 }
             }
             
             // Step 9: Track red nodes
             std::set<Idx> redNodes;
             for(auto& pair : newTree.nodes) {
                 if(pair.second.color == TreeNode::RED) {
                     redNodes.insert(pair.first);
                 }
             }
             
             for(auto& pair : newTree.nodes) {
                 if(pair.second.color != TreeNode::RED) {
                     for(Idx redNode : redNodes) {
                         if(newTree.nodes.find(redNode) != newTree.nodes.end()) {
                             pair.second.rSet.insert(redNode);
                         }
                     }
                 }
             }
             
             // Check if this tree was seen before
             std::string treeSig = ComputeTreeSignature(newTree);
             Idx targetState;
             
             auto stateIt = treeSignatureToState.find(treeSig);
             if(stateIt != treeSignatureToState.end()) {
                 // If so, use existing state
                 targetState = stateIt->second;
                 std::cout << "Debug: Found existing state " << targetState << " for tree" << std::endl;
             } else {
                 // Create new state for this tree
                 targetState = rResGen.InsState();
                 stateToTree[targetState] = newTree;
                 treeSignatureToState[treeSig] = targetState;
                 
                 stateQueue.push(targetState);
                 stateCounter++;
                 
                 std::cout << "Debug: Created new state " << targetState << " for tree" << std::endl;
                 
                 // Set marking based on Rabin acceptance condition
                 bool shouldMark = false;
                 
                 // In this implementation, we mark states that contain green nodes but no red nodes
                 bool hasGreen = false;
                 bool hasRed = false;
                 
                 for(auto& nodePair : newTree.nodes) {
                     if(nodePair.second.color == TreeNode::GREEN) hasGreen = true;
                     if(nodePair.second.color == TreeNode::RED) hasRed = true;
                 }
                 
                 if(hasGreen && !hasRed) {
                     shouldMark = true;
                 }
                 
                 if(shouldMark) {
                     rResGen.SetMarkedState(targetState);
                     std::cout << "Debug: Marking state " << targetState << std::endl;
                 }
             }
             
             // Add transition from current state to target state
             rResGen.SetTransition(currentState, event, targetState);
         }
     }
     
     if(stateCounter >= MAX_STATES) {
         std::cout << "Warning: Reached maximum state limit of " << MAX_STATES << std::endl;
     }
     
     if(iterationCounter >= MAX_ITERATIONS) {
         std::cout << "Warning: Reached maximum iteration limit of " << MAX_ITERATIONS << std::endl;
     }
     
     // Create Rabin pairs for output automaton
     RabinAcceptance outputRabinPairs;
     
     // Create R and I sets based on colors
     StateSet globalR, globalI;
     
     for(auto& statePair : stateToTree) {
         Idx state = statePair.first;
         const LabeledTree& tree = statePair.second;
         
         bool hasRedNode = false;
         bool hasGreenNode = false;
         
         for(auto& nodePair : tree.nodes) {
             if(nodePair.second.color == TreeNode::RED) hasRedNode = true;
             if(nodePair.second.color == TreeNode::GREEN) hasGreenNode = true;
         }
         
         if(hasRedNode) globalR.Insert(state);
         if(hasGreenNode) globalI.Insert(state);
     }
     
     // Add RabinPair when both R and I are non-empty
     if(!globalR.Empty() && !globalI.Empty()) {
         RabinPair newPair;
         newPair.RSet() = globalR;
         newPair.ISet() = globalI;
         outputRabinPairs.Insert(newPair);
     }
     
     rResGen.RabinAcceptance() = outputRabinPairs;
     
     std::cout << "Debug: PseudoDet completed with " 
               << stateCounter << " states and "
               << outputRabinPairs.Size() << " RabinPairs" << std::endl;
               
     return rResGen;
 }
 
 } // namespace faudes