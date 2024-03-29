#ifndef RAGEDGE_H
#define RAGEDGE_H

#include "RagNode.h"
#include <boost/functional/hash.hpp>

namespace NeuroProof {

template <typename Region>
class RagEdge {
  public:
    static RagEdge<Region>* New(RagNode<Region>* node1, RagNode<Region>* node2)
    {
        return new RagEdge(node1, node2);  
    }

    RagNode<Region>* get_node1() const;
    RagNode<Region>* get_node2() const;
    RagNode<Region>* get_other_node(RagNode<Region>* node) const;
    double get_weight() const;
    void set_weight(double weight_);
    void set_edge(RagNode<Region>* region1, RagNode<Region>* region2);


    bool operator<(const RagEdge<Region>& edge2) const;
    bool operator==(const RagEdge<Region>& edge2) const;
    // hash function
    std::size_t operator()(const RagEdge<Region>& edge) const;

    bool is_less(const RagEdge<Region>* edge2) const;
    bool is_equal(const RagEdge<Region>* edge2) const;
    std::size_t compute_hash(const RagEdge<Region>* edge) const;

  private:
    RagEdge(RagNode<Region>* node1_, RagNode<Region>* node_);
    RagNode<Region>* node1;
    RagNode<Region>* node2;
    double weight;
};

template<typename Region> RagEdge<Region>::RagEdge(RagNode<Region>* node1_, RagNode<Region>* node2_) : weight(0.0) 
{
    RagNodePtrCmp<Region> cmp;
    if (cmp(node1_, node2_)) { 
        node1 = node1_;
        node2 = node2_;
    } else {
        node1 = node2_;
        node2 = node1_;
    }
}

template<typename Region> inline RagNode<Region>* RagEdge<Region>::get_node1() const
{
    return node1;
}


template<typename Region> inline RagNode<Region>* RagEdge<Region>::get_node2() const
{
    return node2;
}

template<typename Region> inline RagNode<Region>* RagEdge<Region>::get_other_node(RagNode<Region>* node) const
{
    if (node == node1) {
        return node2;
    } else {
        return node1;
    }
}

template<typename Region> inline double RagEdge<Region>::get_weight() const
{
    return weight;
}

template<typename Region> inline void RagEdge<Region>::set_weight(double weight_)
{
    weight = weight_;
}

template<typename Region> void RagEdge<Region>::set_edge(RagNode<Region>* node1_, RagNode<Region>* node2_) 
{
    RagNodePtrCmp<Region> cmp;
    if (cmp(node1_, node2_)) { 
        node1 = node1_;
        node2 = node2_;
    } else {
        node1 = node2_;
        node2 = node1_;
    }
}

// support functions for overloaded operators and functors
template<typename Region> inline bool RagEdge<Region>::is_less(const RagEdge<Region>* edge2) const
{
    if (node1 < edge2->node1) {
        return true;
    } else if ((node1)->get_node_id() == (edge2->node1)->get_node_id()
            && (node2)->get_node_id() < (edge2->node2)->get_node_id()) {
        return true;
    }
    return false;
}

template<typename Region> inline bool RagEdge<Region>::is_equal(const RagEdge<Region>* edge2) const
{
    return (((node1)->get_node_id() == (edge2->node1)->get_node_id())
            && ((node2)->get_node_id() == (edge2->node2)->get_node_id()));

}

template<typename Region> std::size_t RagEdge<Region>::compute_hash(const RagEdge<Region>* edge) const
{
    std::size_t seed = 0;
    boost::hash_combine(seed, std::size_t((edge->node1)->get_node_id()));
    boost::hash_combine(seed, std::size_t((edge->node2)->get_node_id()));
    return seed;    
}


// overloaded operators
template<typename Region> inline bool RagEdge<Region>::operator<(const RagEdge<Region>& edge2) const
{
    return is_less(&edge2);
}

template<typename Region> inline bool RagEdge<Region>::operator==(const RagEdge<Region>& edge2) const
{
    return is_equal(&edge2);
}

template<typename Region> inline std::size_t RagEdge<Region>::operator()(const RagEdge<Region>& edge) const
{
    return compute_hash(&edge);
}


// functors for operators on pointers
template<typename Region>
struct RagEdgePtrEq {
    bool operator() (const RagEdge<Region>* edge1, const RagEdge<Region>* edge2) const
    {
        return edge1->is_equal(edge2);
    }
};

template<typename Region>
struct RagEdgePtrCmp {
    bool operator() (const RagEdge<Region>* edge1, const RagEdge<Region>* edge2) const
    {
        return edge1->is_less(edge2);
    }
};

template<typename Region>
struct RagEdgePtrHash {
    std::size_t operator() (const RagEdge<Region>* edge2) const
    {
        return  edge2->compute_hash(edge2);
    }
};



}

#endif
