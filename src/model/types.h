#pragma once

#include <map>
#include <string>
#include <vector>

// enum DataType

enum eModelTensorDataType {
  MODEL_TENSOR_DATA_TYPE_UINT8 = 0,
  MODEL_TENSOR_DATA_TYPE_INT8,
  MODEL_TENSOR_DATA_TYPE_UINT16,
  MODEL_TENSOR_DATA_TYPE_INT16,
  MODEL_TENSOR_DATA_TYPE_UINT32,
  MODEL_TENSOR_DATA_TYPE_INT32,
  MODEL_TENSOR_DATA_TYPE_UINT64,
  MODEL_TENSOR_DATA_TYPE_INT64,
  MODEL_TENSOR_DATA_TYPE_FLOAT16,
  MODEL_TENSOR_DATA_TYPE_FLOAT32,
  MODEL_TENSOR_DATA_TYPE_DOUBLE,
  MODEL_TENSOR_DATA_TYPE_BFLOAT16,
  MODEL_TENSOR_DATA_TYPE_BOOL,
  MODEL_TENSOR_DATA_TYPE_STRING,
  MODEL_TENSOR_DATA_TYPE_UNDEFINED = -1,
};

struct sModelGraphNode;
struct sModelGraphEdge;
struct sModelTensor;
struct sModelGraph;

struct sModelTensor {
  // tensor name
  std::string name;
  // tensor shape
  std::vector<int64_t> shape;
  // tensor data type
  enum eModelTensorDataType tensorDataType;
  // this this tensor a constant/initializer/parmeter
  bool is_initializer;
};

struct sModelGraphEdge {
  // tensor name. same as tensor.name
  std::string name;
  // index into sModelGraph::tensors
  int tensor_index = -1;
  // -1 if this is a graph input
  int source_node = -1;
  // -1 if this is a graph output
  int target_node = -1;
};

struct sModelGraphNode {
  // node name
  std::string name;
  // node op type, e.g. Conv, Conv2D, Linear, ReLu, ..
  std::string op_type;
  // node inputs
  std::vector<int> input_edges;
  // node outputs
  std::vector<int> output_edges;
  // node attributes. e.g. kernel_size, stride, ..
  std::map<std::string, std::string> attributes;
};

struct sModelGraph {
  // all logic tensors in the graph
  std::vector<sModelTensor> tensors;
  // all operation nodes
  std::vector<sModelGraphNode> nodes;
  // all edges (connections) between nodes via tensors.
  std::vector<sModelGraphEdge> edges;

  // graph input and output tensors. corresponds to onnx graph, TF placeholder,
  // torch inputs
  std::vector<int> input_tensors;
  std::vector<int> output_tensors;
};
