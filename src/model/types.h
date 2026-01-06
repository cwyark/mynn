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
  MODEL_TENSOR_DATA_TYPE_UNDEFINED = -1,
};

struct sModelTensorDesc {
  // tensor name
  std::string name;
  // tensor shape
  std::vector<int64_t> shape;
  // tensor data type
  enum eModelTensorDataType tensorDataType;
};

struct sModelGraphEdge;
struct sModelGraphNode;

struct sModelGraphNode {
  // node op type, e.g. Conv, Conv2D, Linear, ReLu, ..
  std::string op_type;
  // node name
  std::string name;
  // node inputs
  std::vector<sModelGraphEdge> inputs;
  // node outputs
  std::vector<sModelGraphEdge> outputs;
  // node attributes. e.g. kernel_size, stride, ..
  std::map<std::string, std::string> attributes;
  int64_t param_count;
  double flops_estimate;
};

struct sModelGraphEdge {
  // edge name
  std::string name;
  // edge tensor description
  sModelTensorDesc &tensor_desc;
  // edge source. Must be a valid node.
  sModelGraphNode &source_node;
  // edge tail. Must be a valid node.
  sModelGraphNode &tail_node;
};

struct sModelGraph {
  sModelGraphNode *root_node;
};
