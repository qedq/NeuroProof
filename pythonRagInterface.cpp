#include "DataStructures/Rag.h"
#include "Algorithms/RagAlgs.h"
#include "ImportsExports/ImportExportRagPriority.h"

#include <boost/python.hpp>

#include <boost/numeric/ublas/matrix.hpp>
#include <iostream>

using namespace NeuroProof;
using namespace boost::python;
using std::cout;

typedef Rag<Label> Rag_ui;
typedef RagNode<Label> RagNode_ui;
typedef RagEdge<Label> RagEdge_ui;
        

RagNode<Label>* (Rag_ui::*find_rag_node_ptr)(Label) = &Rag_ui::find_rag_node;
RagNode<Label>* (Rag_ui::*insert_rag_node_ptr)(Label) = &Rag_ui::insert_rag_node;
RagEdge<Label>* (Rag_ui::*insert_rag_edge_ptr)(RagNode<Label>*, RagNode<Label>*) = &Rag_ui::insert_rag_edge;
RagEdge<Label>* (Rag_ui::*find_rag_edge_ptr1)(Label, Label) = &Rag_ui::find_rag_edge;
RagEdge<Label>* (Rag_ui::*find_rag_edge_ptr2)(RagNode<Label>*, RagNode<Label>*) = &Rag_ui::find_rag_edge;


void (*rag_add_property_ptr1)(Rag_ui*, RagEdge_ui*, std::string, object) = rag_add_property<Label, object>;
object (*rag_retrieve_property_ptr1)(Rag_ui*, RagEdge_ui*, std::string) = rag_retrieve_property<Label, object>;
void (*rag_bind_edge_property_list_ptr)(Rag_ui*, std::string) = rag_bind_edge_property_list<Label>;

void (*rag_add_property_ptr2)(Rag_ui*, RagNode_ui*, std::string, object) = rag_add_property<Label, object>;
object (*rag_retrieve_property_ptr2)(Rag_ui*, RagNode_ui*, std::string) = rag_retrieve_property<Label, object>;
void (*rag_bind_node_property_list_ptr)(Rag_ui*, std::string) = rag_bind_node_property_list<Label>;

void (*rag_remove_property_ptr1)(Rag_ui*, RagEdge_ui*, std::string) = rag_remove_property<Label>;
void (*rag_remove_property_ptr2)(Rag_ui*, RagNode_ui*, std::string) = rag_remove_property<Label>;

// add some documentation and produce a simple case (showing that copying works and use of generic properties)

// exception handling??
// allow import/export from json to read location property 
// add a serialize and load rag function -- serialize/deserialize individual properties too (Thrift?)
// add a sort function for default types??
// add specific templates for list and double, etc
// expose property list interface
// make c++ rag algorithms to traverse graph, shortest path, etc


// add specific boost types that allows for specific copies -- how to return templated type unless specific??
// !!pass internal reference already for objects where I want to hold the pointer -- transfer pointer perhaps for other situations like a copy of a Rag -- manage_new_object
// ??still can't pass simple types as references or pointers
// ?? return tuple of pointers??


class RagNode_edgeiterator_wrapper {
  public:
    RagNode_edgeiterator_wrapper(RagNode_ui* rag_node_) : rag_node(rag_node_) {}
    RagNode_ui::edge_iterator edge_begin()
    {
        return rag_node->edge_begin();
    }
    RagNode_ui::edge_iterator edge_end()
    {
        return rag_node->edge_end();
    }
  private:
    RagNode_ui* rag_node;
};

RagNode_edgeiterator_wrapper ragnode_get_edges(RagNode_ui& rag_node) {
    return RagNode_edgeiterator_wrapper(&rag_node);
}


class Rag_edgeiterator_wrapper {
  public:
    Rag_edgeiterator_wrapper(Rag_ui* rag_) : rag(rag_) {}
    Rag_ui::edges_iterator edges_begin()
    {
        return rag->edges_begin();
    }
    Rag_ui::edges_iterator edges_end()
    {
        return rag->edges_end();
    }
  private:
    Rag_ui* rag;
};

Rag_edgeiterator_wrapper rag_get_edges(Rag_ui& rag) {
    return Rag_edgeiterator_wrapper(&rag);
}


class Rag_nodeiterator_wrapper {
  public:
    Rag_nodeiterator_wrapper(Rag_ui* rag_) : rag(rag_) {}
    Rag_ui::nodes_iterator nodes_begin()
    {
        return rag->nodes_begin();
    }
    Rag_ui::nodes_iterator nodes_end()
    {
        return rag->nodes_end();
    }
  private:
    Rag_ui* rag;
};

Rag_nodeiterator_wrapper rag_get_nodes(Rag_ui& rag) {
    return Rag_nodeiterator_wrapper(&rag);
}


// ?! how to add features to RAG
bool build_rag(Rag_ui* rag, object watershed, object prediction)
{
    // there will be 0 padding around image
    unsigned width, height, depth; 

    boost::python::tuple watershed_shape(watershed.attr("shape"));
    boost::python::tuple prediction_shape(prediction.attr("shape"));

    width = boost::python::extract<unsigned>(watershed_shape[0]);
    std::cout << width << std::endl;
    height = boost::python::extract<unsigned>(watershed_shape[1]);
    std::cout << height << std::endl;
    depth = boost::python::extract<unsigned>(watershed_shape[2]);
    std::cout << depth << std::endl;

    unsigned int * watershed_array = new unsigned int[width*height*depth];
    double * prediction_array = new double[width*height*depth];

    unsigned int plane_size = width * height;

    for (unsigned int x = 0; x < width; ++x) {
        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int z = 0; z < depth; ++z) {
                watershed_array[x+y*width+z*plane_size] =
                    int(boost::python::extract<double>(watershed[boost::python::make_tuple(x,y,z)]));
                prediction_array[x+y*width+z*plane_size] =
                    boost::python::extract<double>(prediction[boost::python::make_tuple(x,y,z)]);
            }
        }
    }

    boost::shared_ptr<PropertyList<Label> > edge_list = EdgePropertyList<Label>::create_edge_list();
    rag->bind_property_list("median", edge_list);

    for (unsigned int x = 1; x < (width-1); ++x) {
        for (unsigned int y = 1; y < (height-1); ++y) {
            for (unsigned int z = 1; z < (depth-1); ++z) {
                unsigned long long curr_spot = x + y * width + z * plane_size;
                unsigned int spot0 = watershed_array[curr_spot];
                unsigned int spot1 = watershed_array[curr_spot-1];
                unsigned int spot2 = watershed_array[curr_spot+1];
                unsigned int spot3 = watershed_array[curr_spot-width];
                unsigned int spot4 = watershed_array[curr_spot+width];
                unsigned int spot5 = watershed_array[curr_spot-plane_size];
                unsigned int spot6 = watershed_array[curr_spot+plane_size];

                if (spot0 != spot1) {
                    rag_add_edge(rag, spot0, spot1, prediction_array[curr_spot], edge_list);
                }
                if (spot0 != spot2) {
                    rag_add_edge(rag, spot0, spot1, prediction_array[curr_spot], edge_list);
                }
                if (spot0 != spot3) {
                    rag_add_edge(rag, spot0, spot1, prediction_array[curr_spot], edge_list);
                }
                if (spot0 != spot4) {
                    rag_add_edge(rag, spot0, spot1, prediction_array[curr_spot], edge_list);
                }
                if (spot0 != spot5) {
                    rag_add_edge(rag, spot0, spot1, prediction_array[curr_spot], edge_list);
                }
                if (spot0 != spot6) {
                    rag_add_edge(rag, spot0, spot1, prediction_array[curr_spot], edge_list);
                }

            }
        }
    } 
    return true;
}




BOOST_PYTHON_MODULE(libNeuroProofRag)
{
    // (return: Rag, params: file_name)
    def("create_rag_from_jsonfile", create_rag_from_jsonfile, return_value_policy<manage_new_object>());
    // (return true/false, params: rag, file_name)
    def("create_jsonfile_from_rag", create_jsonfile_from_rag);

    def("build_rag", build_rag);

    // add property to a rag (params: <rag>, <edge/node>, <property_string>, <data>)
    def("rag_add_property", rag_add_property_ptr1);
    def("rag_add_property", rag_add_property_ptr2);

    // remove property from a rag (params: <rag>, <edge/node>, <property_string>)
    def("rag_remove_property", rag_remove_property_ptr1);
    def("rag_remove_property", rag_remove_property_ptr2);
    
    // retrieve property from a rag (return <data>, params: <rag>, <edge/node>, <property_string>)
    def("rag_retrieve_property", rag_retrieve_property_ptr1);
    def("rag_retrieve_property", rag_retrieve_property_ptr2);

    // delete and unbind property list from rag (params: <rag>, <property_string>)
    def("rag_unbind_property_list", rag_unbind_property_list<Label>);

    // create and bind edge property list to rag (params: <rag>, <property_string>)
    def("rag_bind_edge_property_list", rag_bind_edge_property_list_ptr);
    // create and bind node property list to rag (params: <rag>, <property_string>)
    def("rag_bind_node_property_list", rag_bind_node_property_list_ptr);

    // denormalized edge data structure (unique for a node pair)
    class_<RagEdge_ui>("RagEdge", no_init)
        // get first rag node connected to edge
        .def("get_node1", &RagEdge_ui::get_node1, return_value_policy<reference_existing_object>()) 
        // get second rag node connected to edge
        .def("get_node2", &RagEdge_ui::get_node2, return_value_policy<reference_existing_object>()) 
        // returns a double value for the weight
        .def("get_weight", &RagEdge_ui::get_weight) 
        // set a double value for the weight
        .def("set_weight", &RagEdge_ui::set_weight)
        ;
    
    // denormalized node data structure (unique for a node id)
    class_<RagNode_ui>("RagNode", no_init)
        // number of nodes connected to node
        .def("node_degree", &RagNode_ui::node_degree)
        // normalized, unique id for node (node_id, node_id is a normalized id for an edge)
        .def("get_node_id", &RagNode_ui::get_node_id)
        // size as 64 bit unsigned
        .def("get_size", &RagNode_ui::get_size)
        // size as 64 bit unsigned
        .def("set_size", &RagNode_ui::set_size)
        // returns an iterator to the connected edges
        .def("get_edges", ragnode_get_edges)
        // returns an iterator to the connected nodes
        .def("__iter__", range<return_value_policy<reference_existing_object> >(&RagNode_ui::node_begin, &RagNode_ui::node_end))
        ;
 
    class_<Rag_ui>("Rag", init<>())
        // copy constructor supported -- denormolized rag copied correctly, associated properties
        // added in python will just copy the object reference
        .def(init<const Rag_ui&>())
        // returns an interator to the nodes in the rag
        .def("get_edges", rag_get_edges)
        // returns an interator to the edges in the rag
        .def("get_nodes", rag_get_nodes)
        // returns the number of nodes 
        .def("get_num_regions", &Rag_ui::get_num_regions)
        // returns the number of edges 
        .def("get_num_edges", &Rag_ui::get_num_edges)
        // create a new node (return rag_node, params: unique unsigned int) 
        .def("insert_rag_node", insert_rag_node_ptr, return_value_policy<reference_existing_object>())
        // return rag node (return none or node, params: unique unsigned int)
        .def("find_rag_node", find_rag_node_ptr, return_value_policy<reference_existing_object>())
        // return a rag edge (return none or edge, params: node id 1, node id 2)
        .def("find_rag_edge", find_rag_edge_ptr1, return_value_policy<reference_existing_object>())
        // return a rag edge (return none or edge, params: rag node id 1, rag node id 2)
        .def("find_rag_edge", find_rag_edge_ptr2, return_value_policy<reference_existing_object>())
        // insert a rag edge (return none or edge, params: rag node id 1, rag node id 2)
        .def("insert_rag_edge", insert_rag_edge_ptr, return_value_policy<reference_existing_object>())
        // delete rag edge and remove properties associated with edge (params: rag_edge)
        .def("remove_rag_edge", &Rag_ui::remove_rag_edge)
        // delete rag node and connecting edges and remove properties associated with them (params: rag_node)
        .def("remove_rag_node", &Rag_ui::remove_rag_node)
        ;

    // ------- Iterator Interface (should not be explicitly accessed by user) -----------
    // wrapper class for RagNode edge iterator
    class_<RagNode_edgeiterator_wrapper>("RagNode_edgeiterator", no_init)
        .def("__iter__", range<return_value_policy<reference_existing_object> >(&RagNode_edgeiterator_wrapper::edge_begin, &RagNode_edgeiterator_wrapper::edge_end))
        ;
    // wrapper class for Rag edges iterator
    class_<Rag_edgeiterator_wrapper>("Rag_edgeiterator", no_init)
        .def("__iter__", range<return_value_policy<reference_existing_object> >(&Rag_edgeiterator_wrapper::edges_begin, &Rag_edgeiterator_wrapper::edges_end))
        ;
    // wrapper class for Rag nodes iterator
    class_<Rag_nodeiterator_wrapper>("Rag_nodeiterator", no_init)
        .def("__iter__", range<return_value_policy<reference_existing_object> >(&Rag_nodeiterator_wrapper::nodes_begin, &Rag_nodeiterator_wrapper::nodes_end))
        ;
}




/*
stupid& extra_blah(Rag_ui& self, object obj)
{
    stupid& b = (extract<stupid&>(obj));
} 


void* extract_vtk_wrapped_pointer(PyObject* obj)
{
    char thisStr[] = "__this__";
    if (!PyObject_HasAttrString(obj, thisStr))
        return NULL;

    PyObject* thisAttr = PyObject_GetAttrString(obj, thisStr);
    if (thisAttr == NULL)
        return NULL;

    const char* str = PyString_AsString(thisAttr);
    if(str == 0 || strlen(str) < 1)
        return NULL;

    char hex_address[32], *pEnd;
    char *_p_ = strstr(str, "_p_vtk");
    if(_p_ == NULL) return NULL;
    char *class_name = strstr(_p_, "vtk");
    if(class_name == NULL) return NULL;
    strcpy(hex_address, str+1);
    hex_address[_p_-str-1] = '\0';

    long address = strtol(hex_address, &pEnd, 16);

    vtkObjectBase* vtk_object = (vtkObjectBase*)((void*)address);
    if(vtk_object->IsA(class_name))
    {
        return vtk_object;
    }

    return NULL;
}

*/

//#define VTK_PYTHON_CONVERSION(type) \
//    /* register the from-python converter */ \
//    converter::registry::insert(&extract_vtk_wrapped_pointer, type_id<type>());
 



//lvalue_from_pytype<extract_identity<int>,&int>();
//return_internal_reference<1, with_custodian_and_ward<1, 2, with_custodian_and_ward<1, 3> > >());
// copy_const_reference
//object set_object(object obj)
//
//
//
//class_<Base, boost::noncopyable>("Base", no_init);
