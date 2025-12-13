#ifndef FACE_DATABASE_H
#define FACE_DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>

struct PersonRecord {
    int id;
    std::string name;
    int face_count;
    std::string created_at;
    std::string updated_at;
};

struct FaceEmbedding {
    int id;
    int person_id;
    std::string image_path;
    std::vector<unsigned char> embedding_data;  // Serialized face embedding
    std::string created_at;
};

class FaceDatabase {
private:
    sqlite3* db = nullptr;
    std::string db_path = "face_database.db";
    bool is_open = false;

    bool execute_sql(const std::string& sql);
    bool execute_query(const std::string& sql, std::vector<std::map<std::string, std::string>>& results);

public:
    FaceDatabase(const std::string& path = "face_database.db");
    ~FaceDatabase();

    // Database operations
    bool open();
    bool close();
    bool initialize();  // Create tables if they don't exist

    // Person management
    bool add_person(const std::string& name);
    bool get_person(int id, PersonRecord& person);
    bool get_person_by_name(const std::string& name, PersonRecord& person);
    bool get_all_people(std::vector<PersonRecord>& people);
    bool update_person(int id, const std::string& name);
    bool delete_person(int id);
    int get_num_people() const;

    // Face image management
    bool add_face_image(int person_id, const std::string& image_path);
    bool get_face_images(int person_id, std::vector<std::string>& image_paths);
    bool get_all_face_images(std::vector<std::pair<int, std::string>>& images);
    bool delete_face_image(const std::string& image_path);

    // Face embedding management
    bool add_face_embedding(int person_id, const std::string& image_path, const std::vector<unsigned char>& embedding);
    bool get_face_embeddings(int person_id, std::vector<FaceEmbedding>& embeddings);
    bool get_all_face_embeddings(std::vector<FaceEmbedding>& embeddings);
    bool delete_face_embedding(int id);
    bool clear_all_embeddings();  // Clear all embeddings for retraining
    bool update_face_count(int person_id);

    // Query
    bool person_exists(const std::string& name);
    bool is_open_connection() const;

    // Statistics
    int get_total_faces() const;
    int get_faces_per_person(int person_id) const;
};

#endif // FACE_DATABASE_H
