#include <string>

class ModelInspector {
private:
  const std::string &_model_path;

public:
  ModelInspector(const std::string &model_path);
  std::string getName();
  bool load_model(const std::string &model_path);
};
