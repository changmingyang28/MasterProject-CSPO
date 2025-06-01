#include "libfaudes.h"
#include <cmath>
#include <vector>
#include <sstream>
#include <map>
#include <queue>
#include <functional>
#include <stack>
#include <set>

using namespace faudes;

RabinAutomaton PseudoDet(const RabinAutomaton& rGen) {
  FD_DF("PseudoDet(" << rGen.Name() << ")");
  
  // 获取输入Rabin自动机的接受条件
  RabinAcceptance inputRabinPairs = rGen.RabinAcceptance();
  std::cout << "Debug: Input generator has " << inputRabinPairs.Size() << " RabinPairs" << std::endl;
  
  RabinAutomaton rResGen;
  rResGen.Clear();
  rResGen.Name(CollapsString("PseudoDet(" + rGen.Name() + ")"));
  
  if(rGen.InitStatesEmpty()) {
    std::cout << "Debug: Input generator has no initial states, returning empty result." << std::endl;
    return rResGen;
  }
  
  // 复制字母表
  rResGen.InjectAlphabet(rGen.Alphabet());
  
  // 安全限制
  const int MAX_STATES = 1000;
  const int MAX_ITERATIONS = 10000;
  const int MAX_TREE_NODES = 100;
  int stateCounter = 0;
  int iterationCounter = 0;
  
  typedef Idx NodeIdx;
  const Idx INVALID_NODE = 0;
  
  // 标记树节点类
  struct TreeNode {
    StateSet stateLabel;     // S: 状态标签
    std::set<NodeIdx> aSet;  // A-集合
    std::set<NodeIdx> rSet;  // R-集合
    std::vector<NodeIdx> children; // 子节点
    enum Color { WHITE, RED, GREEN } color;  // 用颜色判断Rabin接受条件
    
    TreeNode() : color(WHITE) {}
    
    // 比较运算符
    bool operator<(const TreeNode& other) const {
      if(stateLabel != other.stateLabel) return stateLabel < other.stateLabel;
      if(aSet != other.aSet) return aSet < other.aSet;
      if(rSet != other.rSet) return rSet < other.rSet;
      if(children != other.children) return children < other.children;
      return color < other.color;
    }
    
    bool operator==(const TreeNode& other) const {
      return stateLabel == other.stateLabel && 
             aSet == other.aSet && 
             rSet == other.rSet && 
             children == other.children && 
             color == other.color;
    }
    
    // 调试输出
    std::string ToString() const {
      std::ostringstream oss;
      oss << "Node{states=[" << stateLabel.ToString() << "], ";
      oss << "color=" << (color == WHITE ? "WHITE" : (color == RED ? "RED" : "GREEN")) << ", ";
      oss << "children=" << children.size() << ", ";
      oss << "aSet=" << aSet.size() << ", ";
      oss << "rSet=" << rSet.size() << "}";
      return oss.str();
    }
  };
  
  // 标记树类
  class LabeledTree {
  public:
    std::map<NodeIdx, TreeNode> nodes;
    NodeIdx rootNode;
    NodeIdx nextNodeId;
    
    LabeledTree() : rootNode(INVALID_NODE), nextNodeId(1) {}
    
    NodeIdx createNode() {
      NodeIdx id = nextNodeId++;
      nodes[id] = TreeNode();
      return id;
    }
    
    void deleteNode(NodeIdx nodeId) {
      if(nodes.find(nodeId) == nodes.end()) return;
      
      // 从所有A-集合和R-集合中移除此节点
      for(auto& pair : nodes) {
        pair.second.aSet.erase(nodeId);
        pair.second.rSet.erase(nodeId);
        
        // 从子节点中移除
        auto& children = pair.second.children;
        children.erase(std::remove(children.begin(), children.end(), nodeId), children.end());
      }
      
      // 移除节点本身
      nodes.erase(nodeId);
    }
    
    // 调试输出
    std::string ToString() const {
      std::ostringstream oss;
      oss << "Tree{root=" << rootNode << ", nodes=[";
      for(auto& pair : nodes) {
        oss << pair.first << ":" << pair.second.ToString() << ", ";
      }
      oss << "]}";
      return oss.str();
    }

    bool operator<(const LabeledTree& other) const {
      if(rootNode != other.rootNode) return rootNode < other.rootNode;
      
      // 比较节点数量
      if(nodes.size() != other.nodes.size()) return nodes.size() < other.nodes.size();
      
      // 逐个比较节点
      for(auto it1 = nodes.begin(), it2 = other.nodes.begin();
          it1 != nodes.end() && it2 != other.nodes.end(); ++it1, ++it2) {
        if(it1->first != it2->first) return it1->first < it2->first;
        if(!(it1->second == it2->second)) return it1->second < it2->second;
      }
      
      return false;
    }
    
    bool operator==(const LabeledTree& other) const {
      if(rootNode != other.rootNode) return false;
      if(nodes.size() != other.nodes.size()) return false;
      
      for(auto it1 = nodes.begin(), it2 = other.nodes.begin();
          it1 != nodes.end() && it2 != other.nodes.end(); ++it1, ++it2) {
        if(it1->first != it2->first) return false;
        if(!(it1->second == it2->second)) return false;
      }
      
      return true;
    }
  };
  
  // 存储每个状态对应的树
  std::map<Idx, LabeledTree> stateToTree;
  
  // 跟踪树签名
  std::map<std::string, Idx> treeSignatureToState;
  
  // 计算树的签名
  std::function<std::string(const LabeledTree&)> computeTreeSignature = [](const LabeledTree& tree) {
    std::ostringstream oss;
    
    // 递归处理签名
    std::function<void(NodeIdx)> dfs = [&](NodeIdx nodeId) {
      if(tree.nodes.find(nodeId) == tree.nodes.end()) return;
      
      const TreeNode& node = tree.nodes.at(nodeId);
      oss << nodeId << ":";
      oss << node.stateLabel.ToString() << ":";
      oss << static_cast<int>(node.color) << ":";
      
      // 处理子节点
      for(NodeIdx childId : node.children) {
        dfs(childId);
      }
      
      oss << ";";
    };
    
    dfs(tree.rootNode);
    return oss.str();
  };
  
  // 创建初始树
  LabeledTree initialTree;
  NodeIdx root = initialTree.createNode();
  initialTree.rootNode = root;
  
  // 设置根节点标签包含所有初始状态
  for(StateSet::Iterator sit = rGen.InitStatesBegin(); sit != rGen.InitStatesEnd(); ++sit) {
    initialTree.nodes[root].stateLabel.Insert(*sit);
  }
  
  // 在输出自动机中创建初始状态
  Idx initialState = rResGen.InsInitState();
  stateToTree[initialState] = initialTree;
  
  std::string initialSig = computeTreeSignature(initialTree);
  treeSignatureToState[initialSig] = initialState;
  
  std::queue<Idx> stateQueue;
  stateQueue.push(initialState);
  stateCounter++;
  
  // 处理所有状态
  while(!stateQueue.empty() && stateCounter < MAX_STATES && iterationCounter < MAX_ITERATIONS) {
    iterationCounter++;
    
    Idx currentState = stateQueue.front();
    stateQueue.pop();
    
    LabeledTree currentTree = stateToTree[currentState];
    std::cout << "Debug: Processing state " << currentState << " with tree: " 
              << currentTree.ToString() << std::endl;
    
    // 处理每个事件
    for(EventSet::Iterator evIt = rGen.AlphabetBegin(); evIt != rGen.AlphabetEnd(); ++evIt) {
      Idx event = *evIt;
      std::cout << "Debug: Processing event " << rGen.EventName(event) << std::endl;
      
      // 克隆树
      LabeledTree newTree = currentTree;
      
      // 如果树太大，跳过
      if(newTree.nodes.size() > MAX_TREE_NODES) {
        std::cout << "Warning: Tree size exceeds limit, skipping event." << std::endl;
        continue;
      }
      
      // 步骤1：将所有节点着色为白色
      for(auto& pair : newTree.nodes) {
        pair.second.color = TreeNode::WHITE;
      }
      
      // 步骤2：基于转移更新状态标签
      for(auto& pair : newTree.nodes) {
        NodeIdx nodeId = pair.first;
        TreeNode& node = pair.second;
        
        // 基于后继状态创建新标签
        StateSet newLabel;
        
        // 检查是否为epsilon事件
        bool isEpsilonEvent = false;
        std::string eventName = rGen.EventName(event);
        if(eventName == "eps" || eventName == "epsilon") {
          isEpsilonEvent = true;
        }
        
        if(isEpsilonEvent) {
          // 对于epsilon转移: δ(ε, Y) ∪ Y
          // 首先包含原始状态集合 Y
          for(StateSet::Iterator sit = node.stateLabel.Begin(); sit != node.stateLabel.End(); ++sit) {
            newLabel.Insert(*sit);
          }
          
          // 然后添加通过epsilon转移可达的状态 δ(ε, Y)
          for(StateSet::Iterator sit = node.stateLabel.Begin(); sit != node.stateLabel.End(); ++sit) {
            Idx state = *sit;
            
            for(TransSet::Iterator tit = rGen.TransRelBegin(state, event); 
                tit != rGen.TransRelEnd(state, event); ++tit) {
              newLabel.Insert(tit->X2);
            }
          }
        } else {
          // 对于普通事件: δ(σ, Y)
          // 检查从此节点和事件的所有转移
          for(StateSet::Iterator sit = node.stateLabel.Begin(); sit != node.stateLabel.End(); ++sit) {
            Idx state = *sit;
            
            for(TransSet::Iterator tit = rGen.TransRelBegin(state, event); 
                tit != rGen.TransRelEnd(state, event); ++tit) {
              newLabel.Insert(tit->X2);
            }
          }
        }
        
        // 更新节点的状态标签
        node.stateLabel = newLabel;
      }
      
      // 步骤3：为潜在的接受违规创建节点
      if(inputRabinPairs.Size() > 0) {
        // 对总子节点数的硬限制
        const int MAX_TOTAL_CHILDREN = 40;
        int totalChildrenCreated = 0;
        
        for(auto& pair : newTree.nodes) {
          NodeIdx nodeId = pair.first;
          TreeNode& node = pair.second;
          
          // 如果创建了太多子节点，跳出整个循环
          if(totalChildrenCreated >= MAX_TOTAL_CHILDREN) {
            std::cout << "Warning: Reached maximum total children limit." << std::endl;
            break;
          }
          
          // 每个节点的限制
          const int MAX_CHILDREN_PER_NODE = 3;
          int nodeChildrenCount = 0;
          
          // 对每个Rabin对
          RabinAcceptance::CIterator rit;
          Idx i = 0;
          for(rit = inputRabinPairs.Begin(); 
              rit != inputRabinPairs.End() && nodeChildrenCount < MAX_CHILDREN_PER_NODE; 
              ++rit, ++i) {
            
            // 检查I集合之外的状态
            StateSet outsideI;
            const StateSet& I = rit->ISet();
            
            for(StateSet::Iterator sit = node.stateLabel.Begin(); 
                sit != node.stateLabel.End() && outsideI.Size() < 5; ++sit) {
              if(!I.Exists(*sit)) {
                outsideI.Insert(*sit);
              }
            }
            
            // 只有当我们有I之外的状态且未超过限制时才创建子节点
            if(!outsideI.Empty() && nodeChildrenCount < MAX_CHILDREN_PER_NODE) {
              // 检查此标签是否已在现有子节点中表示
              bool alreadyRepresented = false;
              for(NodeIdx childId : node.children) {
                if(newTree.nodes.find(childId) != newTree.nodes.end()) {
                  // 如果有显著重叠，认为标签等价
                  StateSet intersection = newTree.nodes[childId].stateLabel * outsideI;
                  if(!intersection.Empty()) {
                    alreadyRepresented = true;
                    break;
                  }
                }
              }
              
              if(!alreadyRepresented) {
                NodeIdx newChild = newTree.createNode();
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
      
      // 步骤4：维护兄弟节点间的状态不相交性
      for(auto& pair : newTree.nodes) {
        NodeIdx parentId = pair.first;
        TreeNode& parent = pair.second;
        
        // 子节点已按年龄排序（假设节点ID随创建时间递增）
        // 对每个子节点，移除出现在较老兄弟节点中的状态
        for(size_t i = 1; i < parent.children.size(); ++i) {
          NodeIdx youngerId = parent.children[i];
          
          if(newTree.nodes.find(youngerId) == newTree.nodes.end()) continue;
          
          for(size_t j = 0; j < i; ++j) {
            NodeIdx olderId = parent.children[j];
            
            if(newTree.nodes.find(olderId) == newTree.nodes.end()) continue;
            
            // 从年轻节点中移除较老兄弟节点的所有状态
            for(StateSet::Iterator sit = newTree.nodes[olderId].stateLabel.Begin(); 
                sit != newTree.nodes[olderId].stateLabel.End(); ++sit) {
              newTree.nodes[youngerId].stateLabel.Erase(*sit);
            }
          }
        }
      }
      
      // 步骤5：移除所有空状态标签的节点
      std::vector<NodeIdx> nodesToRemove;
      for(auto& pair : newTree.nodes) {
        if(pair.second.stateLabel.Empty()) {
          nodesToRemove.push_back(pair.first);
        }
      }
      
      for(NodeIdx nodeId : nodesToRemove) {
        newTree.deleteNode(nodeId);
      }
      
      // 步骤6：判断红色断点
      for(auto& pair : newTree.nodes) {
        NodeIdx nodeId = pair.first;
        TreeNode& node = pair.second;
        
        // 计算子节点状态标签的并集
        StateSet unionOfChildren;
        for(NodeIdx childId : node.children) {
          if(newTree.nodes.find(childId) == newTree.nodes.end()) continue;
          
          for(StateSet::Iterator sit = newTree.nodes[childId].stateLabel.Begin(); 
              sit != newTree.nodes[childId].stateLabel.End(); ++sit) {
            unionOfChildren.Insert(*sit);
          }
        }
        
        if(node.stateLabel == unionOfChildren && !unionOfChildren.Empty()) {
          std::cout << "Debug: Red breakpoint at node " << nodeId << std::endl;
          node.color = TreeNode::RED;
          
          // 删除所有后代
          std::vector<NodeIdx> descendants;
          std::queue<NodeIdx> bfs;
          for(NodeIdx child : node.children) {
            if(newTree.nodes.find(child) != newTree.nodes.end()) {
              bfs.push(child);
            }
          }
          
          while(!bfs.empty()) {
            NodeIdx current = bfs.front();
            bfs.pop();
            descendants.push_back(current);
            
            if(newTree.nodes.find(current) == newTree.nodes.end()) continue;
            
            for(NodeIdx child : newTree.nodes[current].children) {
              if(newTree.nodes.find(child) != newTree.nodes.end()) {
                bfs.push(child);
              }
            }
          }
          
          for(NodeIdx descendant : descendants) {
            newTree.deleteNode(descendant);
          }
          
          node.children.clear();
          node.aSet.clear();
          node.rSet.clear();
        }
      }
      
      // 步骤7：更新A-集合和R-集合
      // 移除在步骤5和步骤6中删除的节点
      for(auto& pair : newTree.nodes) {
        auto& aSet = pair.second.aSet;
        auto& rSet = pair.second.rSet;
        
        std::vector<NodeIdx> toRemoveFromA, toRemoveFromR;
        
        for(NodeIdx nodeInA : aSet) {
          if(newTree.nodes.find(nodeInA) == newTree.nodes.end()) {
            toRemoveFromA.push_back(nodeInA);
          }
        }
        
        for(NodeIdx nodeInR : rSet) {
          if(newTree.nodes.find(nodeInR) == newTree.nodes.end()) {
            toRemoveFromR.push_back(nodeInR);
          }
        }
        
        for(NodeIdx nodeId : toRemoveFromA) aSet.erase(nodeId);
        for(NodeIdx nodeId : toRemoveFromR) rSet.erase(nodeId);
      }
      
      // 步骤8：处理绿色着色
      for(auto& pair : newTree.nodes) {
        NodeIdx nodeId = pair.first;
        TreeNode& node = pair.second;
        
        if(node.aSet.empty() && node.color != TreeNode::RED) {
          std::cout << "Debug: Green coloring for node " << nodeId << std::endl;
          node.color = TreeNode::GREEN;
          node.aSet = node.rSet;
          node.rSet.clear();
        }
      }
      
      // 步骤9：跟踪红色节点
      std::set<NodeIdx> redNodes;
      for(auto& pair : newTree.nodes) {
        if(pair.second.color == TreeNode::RED) {
          redNodes.insert(pair.first);
        }
      }
      
      for(auto& pair : newTree.nodes) {
        if(pair.second.color != TreeNode::RED) {
          for(NodeIdx redNode : redNodes) {
            if(newTree.nodes.find(redNode) != newTree.nodes.end()) {
              pair.second.rSet.insert(redNode);
            }
          }
        }
      }
      
      // 检查此树是否之前见过
      std::string treeSig = computeTreeSignature(newTree);
      Idx targetState;
      
      auto stateIt = treeSignatureToState.find(treeSig);
      if(stateIt != treeSignatureToState.end()) {
        // 如果是，使用现有状态
        targetState = stateIt->second;
        std::cout << "Debug: Found existing state " << targetState << " for tree" << std::endl;
      } else {
        // 为此树创建新状态
        targetState = rResGen.InsState();
        stateToTree[targetState] = newTree;
        treeSignatureToState[treeSig] = targetState;
        
        stateQueue.push(targetState);
        stateCounter++;
        
        std::cout << "Debug: Created new state " << targetState << " for tree" << std::endl;
        
        // 根据Rabin接受条件设置标记
        bool shouldMark = false;
        
        // 在此实现中，我们标记那些包含绿色节点但不包含红色节点的状态
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
      
      // 添加从当前状态到目标状态的转移
      rResGen.SetTransition(currentState, event, targetState);
    }
  }
  
  if(stateCounter >= MAX_STATES) {
    std::cout << "Warning: Reached maximum state limit of " << MAX_STATES << std::endl;
  }
  
  if(iterationCounter >= MAX_ITERATIONS) {
    std::cout << "Warning: Reached maximum iteration limit of " << MAX_ITERATIONS << std::endl;
  }
  
  // 为输出自动机创建Rabin对
  RabinAcceptance outputRabinPairs;
  
  // 基于颜色创建R和I集合
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
  
  // 当R和I都非空时添加RabinPair
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

int main(void) {
    RabinAutomaton NRA;
    NRA.Read("pseudotest/test3.gen");
    NRA.DWrite();
    
    RabinAutomaton DRA = PseudoDet(NRA);
    DRA.DWrite();
    
    return 0;
}