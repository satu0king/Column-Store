#include "DataLoader.h"
#include "SchemaLoader.h"
#include "Table.h"
#include "stdlib.h"

int main() {
    // system(XMLVALIDATE);
    SchemaExtractor schema_extractor("../schema.xml");
    SchemaMetaData schema_meta_data = schema_extractor.get_meta_data();
    SchemaLoader schema_loader("column_store", "test", "test",
                               schema_meta_data);
    DataLoader data_loader("column_store", "test", "test",
                           "../data/dummy_data.sql");
    return 0;
}
