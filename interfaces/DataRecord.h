#pragma once;

template <typename T>
class DataValue {
    T value;

   public:
    DataValue(T value) : value(value) {}

    template <typename V>
    T as<V>() {
        assert(V == T);
        return value;
    }
};

class DataRecord {
    std::vector<DataValue> values;
};

