#pragma once

#include <stdio.h>
#include <string.h>

#include <fstream>
#include <string>
#include <vector>

#include "Parser/Projection.h"
#include "interfaces/DataRecord.h"

namespace ColumnStore {

struct ColumnStoreData {
    std::vector<unsigned char> data;
    ColumnStoreData(std::vector<unsigned char> &data) : data(data) {}
    ColumnStoreData(std::vector<unsigned char> &&data) : data(data) {}
    ColumnStoreData(ColumnStoreData &&data) : data(data.data) {}
    ColumnStoreData(int size) : data(size) {}

    ColumnStoreData(DataRecord &record, std::vector<Parser::Column> &columns) {
        int size = 0;
        for (auto &c : columns) size += c;

        data.resize(size);
        set(record, columns);
    }

    float getFloat(int i) {
        return *reinterpret_cast<float *>(data.data() + i);
    }

    int getInt(int i) { return *reinterpret_cast<int *>(data.data() + i); }

    std::string getString(int i, int length) {
        return std::string(data.data() + i, data.data() + i + length);
    }

    void setFloat(int i, float value) {
        *reinterpret_cast<float *>(data.data() + i) = value;
    }

    void setInt(int i, int value) {
        *reinterpret_cast<int *>(data.data() + i) = value;
    }

    void setString(int i, std::string string) {
        memcpy(data.data() + i, string.c_str(), string.size());
    }

    int size() const { return data.size(); }

    operator const char *() const { return (const char *)data.data(); }
    operator char *() { return (char *)data.data(); }

    friend std::ostream &operator<<(std::ostream &output,
                                    const ColumnStoreData &data) {
        output.write(data, data.size());
        return output;
    }

    friend std::istream &operator>>(std::istream &input,
                                    ColumnStoreData &data) {
        input.read(data, data.size());
        return input;
    }

    void read(std::istream &input) { input.read(*this, data.size()); }

    void write(std::ostream &output) { output.write(*this, data.size()); }

    DataRecord get(std::vector<Parser::Column> &columns) {
        std::vector<DataValue> values;
        values.reserve(columns.size());

        int offset = 0;
        for (int i = 0; i < columns.size(); ++i) {
            Parser::DataType type = columns[i];
            if (type == ColumnStore::DataType::INT) {
                values.push_back(DataValue(getInt(offset)));
                offset += 4;
            } else if (type == ColumnStore::DataType::FLOAT) {
                values.push_back(DataValue(getFloat(offset)));
                offset += 4;
            } else if (type == ColumnStore::DataType::STRING) {
                values.push_back(DataValue(getString(offset, type.size)));
                offset += type.size;
            } else
                throw std::runtime_error("Unknown DataType");
        }
        return DataRecord(values);
    }

    void set(DataRecord &record, std::vector<Parser::Column> &columns) {
        int offset = 0;
        for (int i = 0; i < columns.size(); ++i) {
            Parser::DataType type = columns[i];
            if (type == ColumnStore::DataType::INT) {
                setInt(offset, record[i].as<int>());
                offset += 4;
            } else if (type == ColumnStore::DataType::FLOAT) {
                setFloat(offset, record[i].as<float>());
                offset += 4;
            } else if (type == ColumnStore::DataType::STRING) {
                setString(offset, record[i].as<std::string>());
                offset += type.size;
            } else
                throw std::runtime_error("Unknown DataType");
        }
    }
};

};  // namespace ColumnStore