#ifndef FAISS_INDEX_H
#define FAISS_INDEX_H

#include <vector>
#include <map>
#include <string>
#include <memory>

// Forward declare FAISS opaque pointer types
typedef struct FaissIndex FaissIndex;

class FAISSIndex {
private:
    void* index = nullptr;  // Opaque pointer (not used in simple implementation)
    std::vector<int> person_ids;  // Maps FAISS vector index to person_id
    std::vector<std::vector<float>> embeddings;  // In-memory embedding storage
    int dimension = 128;
    int num_clusters = 0;
    bool is_built = false;

    // Helper for normalized L2 distance to similarity
    double distance_to_similarity(float distance) const;

public:
    FAISSIndex(int embedding_dimension = 128);
    ~FAISSIndex();

    // Index management
    bool build_index(int num_vectors);
    bool add_vector(int person_id, const std::vector<float>& embedding);
    bool add_vectors(const std::vector<int>& ids, const std::vector<std::vector<float>>& embeddings);

    // Search
    // Returns person_id of nearest neighbor and confidence (0-1)
    int search(const std::vector<float>& query_embedding, double& confidence);
    std::vector<int> search_k(const std::vector<float>& query_embedding, int k, std::vector<double>& confidences);

    // Persistence
    bool save_index(const std::string& filepath);
    bool load_index(const std::string& filepath);

    // State
    bool is_index_built() const { return is_built; }
    int get_num_vectors() const;
    int get_dimension() const { return dimension; }
    int get_num_clusters() const { return num_clusters; }
    void clear();

private:
    // FAISS helper methods
    int calculate_optimal_clusters(int num_vectors);
    void setup_index_parameters();
};

#endif // FAISS_INDEX_H
