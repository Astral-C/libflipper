#pragma once

#include "GXGeometryEnums.hpp"
#include "GXVertexData.hpp"

#include <cstdint>
#include <vector>
#include <memory>

class GXGeometry;

// Represents a single primitive made up of a list of vertices.
class GXPrimitive {
    // What kind of shape the vertices in this primitive make - triangles, quads, etc.
    EGXPrimitiveType mType;
    // The vertices making up this primitive.
    std::vector<ModernVertex> mVertices;

    // Converts this primitive from triangle strip to triangles.
    void TriangulateTriangleStrip();
    // Converts this primitive from triangl fan to triangles.
    void TriangulateTriangleFan();

public:
    GXPrimitive() : mType(EGXPrimitiveType::None) {}
    GXPrimitive(const EGXPrimitiveType& type) { mType = type; }

    // Returns this primitive's type.
    EGXPrimitiveType GetType() const { return mType; }

    // Returns a reference to this primitive's list of vertices.
    std::vector<ModernVertex>& GetVertices() { return mVertices; }
    // Returns a const reference to this primitive's list of vertices.
    const std::vector<ModernVertex>& GetVertices() const { return mVertices; }

    // Reconfigures the indices in this primitive from whatever its
    // original primitive type was to triangles.
    void TriangluatePrimitive();
};


// Represents a set of primitives sharing the same Vertex Attribute Table setup,
// i.e. a set of primitives with the same attributes enabled.
class GXShape {
    friend GXGeometry;

    // A list that indicates which attributes are enabled for the primitives in this shape.
    std::vector<EGXAttribute> mVertexAttributeTable;
    // The primitives that make up this shape.
    std::vector<GXPrimitive*> mPrimitives;

    std::vector<ModernVertex> mVertices;

    // The offset of this shape's first vertex index in the model index list.
    uint32_t mFirstVertexOffset;
    // The total number of vertex indices that this shape has in the model index list.
    uint32_t mVertexCount;

    glm::vec3 mCenterOfMass;

    bool mbIsVisible;

    // Arbitrary data that can be associated with this shape.
    void* mUserData;

public:
    GXShape() : mFirstVertexOffset(0), mVertexCount(0), mCenterOfMass(), mbIsVisible(true), mUserData(nullptr) {}

    ~GXShape() {
        for (GXPrimitive* p : mPrimitives) {
            delete p;
        }

        mPrimitives.clear();
        delete mUserData;
    }

    // Returns a reference to this shape's list of enabled attributes.
    std::vector<EGXAttribute>& GetAttributeTable() { return mVertexAttributeTable; }
    // Returns a reference to this shape's list of primitives.
    std::vector<GXPrimitive*>& GetPrimitives() { return mPrimitives; }
    
    // Returns a const reference to this shape's list of enabled attributes.
    const std::vector<EGXAttribute>& GetAttributeTable() const { return mVertexAttributeTable; }
    // Returns a const reference to this shape's list of primitives.
    const std::vector<GXPrimitive*>& GetPrimitives() const { return mPrimitives; }

    const glm::vec3& GetCenterOfMass() const { return mCenterOfMass; }

    void SetVertexOffset(uint32_t offset) { mFirstVertexOffset = offset; }
    // Fills the input references with the offset of this shape's first index in the global index list
    // and the number of indices belonging to it.
    void GetVertexOffsetAndCount(uint32_t& offset, uint32_t& count) const;

    bool GetVisible() const { return mbIsVisible; }
    void SetVisible(bool visible) { mbIsVisible = visible; }

    void* GetUserData() const { return mUserData; }

    template<typename T>
    T* GetUserData() const { return static_cast<T*>(mUserData); }

    void SetUserData(void* data) { mUserData = data; }

    void CalculateCenterOfMass();
};

// Represents all of the geometry for a given model.
class GXGeometry {
    // The geometry data that makes up this model.
    std::vector<std::shared_ptr<GXShape>> mShapes;

    // All the vertex indices in the model, collated for one-and-done uploading to the GPU.
    std::vector<uint32_t> mModelIndices;
    // All the vertex data in the model, sorted by the model's indices.
    std::vector<ModernVertex> mModelVertices;

public:
    GXGeometry() { }

    ~GXGeometry() {
        mShapes.clear();
    }

    uint32_t AddVertices(std::vector<ModernVertex> vertices);

    // Returns a reference to the list of shapes in this model.
    std::vector<std::shared_ptr<GXShape>>& GetShapes() { return mShapes; }
    // Returns a reference to the list of all vertex indices in this model.
    std::vector<uint32_t>& GetModelIndices() { return mModelIndices; }
    // Returns a reference to the list of all vertices in this model.
    std::vector<ModernVertex>& GetModelVertices() { return mModelVertices; }

    // Returns a const reference to the list of shapes in this model.
    const std::vector<std::shared_ptr<GXShape>>& GetShapes() const { return mShapes; }
    // Returns a const reference to the list of all vertex indices in this model.
    const std::vector<uint32_t>& GetModelIndices() const { return mModelIndices; }
    // Returns a const reference to the list of all vertices in this model.
    const std::vector<ModernVertex>& GetModelVertices() const { return mModelVertices; }

    void CleanupVertexArray() {
        mModelVertices = {};
        mModelIndices = {};

        for(auto shape : mShapes){
            for (GXPrimitive* p : shape->GetPrimitives()) {
                delete p;
            }

            shape->GetPrimitives().clear();
        }

    }

    // Processes the loaded geometry to be easier for modern GPUs to render.
    void CreateVertexArray();
};
