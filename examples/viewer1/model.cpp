#include "model.hpp"

#include <fmt/core.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <filesystem>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

// Custom specialization of std::hash injected in namespace std
namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const noexcept {
    std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};
}  // namespace std

Model::~Model() {
  glDeleteBuffers(1, &m_EBO);
  glDeleteBuffers(1, &m_VBO);
  glDeleteVertexArrays(1, &m_VAO);
}

void Model::createBuffers() {
  // Delete previous buffers
  glDeleteBuffers(1, &m_EBO);
  glDeleteBuffers(1, &m_VBO);

  // VBO
  glGenBuffers(1, &m_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(),
               m_vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // EBO
  glGenBuffers(1, &m_EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * m_indices.size(),
               m_indices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Model::loadFromFile(std::string_view path, bool standardize) {
  auto basePath{std::filesystem::path{path}.parent_path().string() + "/"};

  tinyobj::ObjReaderConfig readerConfig;
  readerConfig.mtl_search_path = basePath;  // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data(), readerConfig)) {
    if (!reader.Error().empty()) {
      throw abcg::Exception{abcg::Exception::Runtime(
          fmt::format("Failed to load model {} ({})", path, reader.Error()))};
    }
    throw abcg::Exception{
        abcg::Exception::Runtime(fmt::format("Failed to load model {}", path))};
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  const auto& attrib{reader.GetAttrib()};
  const auto& shapes{reader.GetShapes()};

  m_vertices.clear();
  m_indices.clear();

  // A key:value map with key=Vertex and value=index
  std::unordered_map<Vertex, GLuint> hash{};

  // Loop over shapes
  for (const auto& shape : shapes) {
    // Loop over indices
    for (const auto offset : iter::range(shape.mesh.indices.size())) {
      // Access to vertex
      tinyobj::index_t index{shape.mesh.indices.at(offset)};

      // Vertex coordinates
      std::size_t startIndex{static_cast<size_t>(3 * index.vertex_index)};
      float vx{attrib.vertices.at(startIndex + 0)};
      float vy{attrib.vertices.at(startIndex + 1)};
      float vz{attrib.vertices.at(startIndex + 2)};

      Vertex vertex{};
      vertex.position = {vx, vy, vz};

      // If hash doesn't contain this vertex
      if (hash.count(vertex) == 0) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      m_indices.push_back(hash[vertex]);
    }
  }

  if (standardize) {
    this->standardize();
  }

  createBuffers();
}

void Model::render(int numTriangles) const {
  glBindVertexArray(m_VAO);

  GLsizei numIndices = (numTriangles < 0) ? m_indices.size() : numTriangles * 3;

  glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
}

void Model::setupVAO(GLuint program) {
  // Release previous VAO
  glDeleteVertexArrays(1, &m_VAO);

  // Create VAO
  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);

  // Bind EBO and VBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Bind vertex attributes
  GLint positionAttribute = glGetAttribLocation(program, "inPosition");
  if (positionAttribute >= 0) {
    glEnableVertexAttribArray(positionAttribute);
    glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), nullptr);
  }

  // End of binding
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Model::standardize() {
  // Center to origin and normalize largest bound to [-1, 1]

  // Get bounds
  glm::vec3 max(std::numeric_limits<float>::lowest());
  glm::vec3 min(std::numeric_limits<float>::max());
  for (const auto& vertex : m_vertices) {
    max.x = std::max(max.x, vertex.position.x);
    max.y = std::max(max.y, vertex.position.y);
    max.z = std::max(max.z, vertex.position.z);
    min.x = std::min(min.x, vertex.position.x);
    min.y = std::min(min.y, vertex.position.y);
    min.z = std::min(min.z, vertex.position.z);
  }

  // Center and scale
  const auto center{(min + max) / 2.0f};
  const auto scaling{2.0f / glm::length(max - min)};
  for (auto& vertex : m_vertices) {
    vertex.position = (vertex.position - center) * scaling;
  }
}
