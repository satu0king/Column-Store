#include <boost/program_options.hpp>
#include <iostream>

#include "PostgresDataGenerator/PostgreSQLDataGenerator.h"
#include "PostgresDataGenerator/PostgreSQLMetaData.h"
#include "ProjectionData.h"

namespace po = boost::program_options;

po::variables_map config;

void initConfig(int ac, char *av[]) {
    try {
        po::options_description desc("Allowed options");
        auto init = desc.add_options();

        init("column_store_path", po::value<std::string>(),
             "Directory to initialize column store");

        po::store(po::parse_command_line(ac, av, desc), config);
        po::notify(config);

        if (config.count("help")) {
            std::cout << desc << "\n";
            exit(0);
        }

        if (!config.count("column_store_path")) {
            std::cout << "Column store path necessary\n";
            std::cout << desc << "\n";
            exit(EXIT_FAILURE);
        }
    } catch (std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cerr << "Exception of unknown type!\n";
        exit(EXIT_FAILURE);
    }
}

using namespace ColumnStore;

int main(int argc, char *argv[]) {
    initConfig(argc, argv);

    ColumnStore::ColStoreLoader loader(
        config["column_store_path"].as<std::string>());
    loader.updateAll();
}