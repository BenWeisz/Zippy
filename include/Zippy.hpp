#pragma once

#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>

#include <zip.h>

/* Zip up a folder into a .zip file:
    inputPath: A relative path to folder to be zipped up
    outputPath: The name of the output zip

    return: true if the zipping process was successful, false otherwise

    The zip will be placed in the same folder as the executable
    inputPath is relative to the executable.
    If the folder was already zipped, the old zip will first be deleted.
*/
bool zippy_up(const std::string& inputPath, const std::string& outputPath) {
    // We will only accept relative paths that don't start with a /
    if (inputPath.starts_with("/") || inputPath.starts_with("\\")) {
        std::cout << "ERROR: Input path must be a relative path that doesn't start with a \"/\" character." << std::endl;
        return false;
    }
    if (outputPath.starts_with("/") || outputPath.starts_with("\\")) {
        std::cout << "ERROR: Output path must be a relative path that doesn't start with a \"/\" character." << std::endl;
        return false;
    }

    // Only accept output files that are .zip
    if (!outputPath.ends_with(".zip")) {
        std::cout << "ERROR: Output target name must be of type .zip" << std::endl;
        return false;
    }

    std::string dataPath = std::filesystem::current_path() / inputPath;
    const std::string zipPath = std::filesystem::current_path() / outputPath;

    if (!std::filesystem::exists(dataPath)) {
        std::cout << "ERROR: The data folder: \"" << dataPath << "\" does not exist" << std::endl;
        return false;
    }

    if (std::filesystem::exists(zipPath)) {
        std::cout << "LOG: Removing existing zip file: \"" << zipPath << "\"" << std::endl;
        std::filesystem::remove(zipPath);
    }

    if (!dataPath.ends_with("/") && !dataPath.ends_with("\""))
        dataPath += "/";

    int dataPathLen = dataPath.length();

    // Open a new zip file for writing
    int ignore;
    zip_t* zipFile = zip_open(zipPath.c_str(), ZIP_CREATE, &ignore);
    if (zipFile == nullptr) {
        std::cout << "ERROR: Failed to create zip file: \"" << zipPath << "\"" << std::endl;
        return false;
    }

    zip_int64_t status;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dataPath)) {
        const std::string entryPath = entry.path().string();
        const std::string relativeEntryPath = entryPath.substr(dataPathLen - 1);

        // Add a regular file to the archive
        if (std::filesystem::is_regular_file(entryPath)) {
            zip_source_t* entrySource = zip_source_file(zipFile, entryPath.c_str(), 0, 0);
            if (entrySource == nullptr) {
                const char* strError = zip_strerror(zipFile);
                std::cout << "ERROR: Failed to create file source for: \"" << relativeEntryPath << "\"" << std::endl;
                std::cout << "\t" << strError << std::endl;

                zip_discard(zipFile);
                return false;
            }

            // Add source file to the zip file
            status = zip_file_add(zipFile, relativeEntryPath.c_str(), entrySource, ZIP_FL_OVERWRITE);
            if (status < 0) {
                const char* strError = zip_strerror(zipFile);
                std::cout << "ERROR: Failed to add file: \"" << relativeEntryPath << "\" to zip file" << std::endl;
                std::cout << "\t" << strError << std::endl;

                zip_discard(zipFile);
                return false;
            }
        } else if (std::filesystem::is_directory(entryPath)) {
            status = zip_dir_add(zipFile, relativeEntryPath.c_str(), ZIP_FL_ENC_GUESS);
            if (status < 0) {
                const char* strError = zip_strerror(zipFile);
                std::cout << "ERROR: Failed to add directory: \"" << relativeEntryPath << "\" to zip file" << std::endl;
                std::cout << "\t" << strError << std::endl;

                zip_discard(zipFile);
                return false;
            }
        }
    }

    // Close the zip file
    int err = zip_close(zipFile);
    if (err != 0) {
        const char* strError = zip_strerror(zipFile);
        std::cout << "ERROR: Failed to save the zip file: \"" << outputPath << "\"" << std::endl;
        std::cout << "\t" << strError << std::endl;

        zip_discard(zipFile);
        return false;
    }

    return true;
}

/* Unzip a zip file:
    inputPath: The relative path of the zip file to unzip

    return: true if the unzipping process was successful, false otherwise

    inputPath is relative to the executable. The file will be unzipped to
    the same folder as the executable with the same name as the zip file.
    If the zip file was already unzipped its old unzipped folder will first be deleted.
*/
bool zippy_down(const std::string& inputPath) {
    if (!std::filesystem::exists(inputPath)) {
        std::cout << "ERROR: The zip file: \"" << inputPath << "\" does not exist" << std::endl;
        return false;
    }

    if (inputPath.starts_with("/") || inputPath.starts_with("\\")) {
        std::cout << "ERROR: Zip path must be a relative path that doesn't start with a \"/\" character." << std::endl;
        return false;
    }

    // Only accept output files that are .zip
    if (!inputPath.ends_with(".zip")) {
        std::cout << "ERROR: Zip file must be of type .zip" << std::endl;
        return false;
    }

    const std::string zipPath = std::filesystem::current_path() / inputPath;

    int ignore;
    zip_t* zipFile = zip_open(zipPath.c_str(), ZIP_RDONLY, &ignore);
    if (zipFile == nullptr) {
        std::cout << "ERROR: Failed to create zip file: \"" << zipPath << "\"" << std::endl;
        return false;
    }

    zip_int64_t numEntries = zip_get_num_entries(zipFile, ZIP_FL_UNCHANGED);
    if (numEntries < 0) {
        std::cout << "ERROR: The zip file \"" << zipPath << "\" contains no files" << std::endl;
        zip_discard(zipFile);

        return false;
    }

    // Make the folder for the unzipped files
    const std::string folderPath = zipPath.substr(0, zipPath.length() - 4);
    if (std::filesystem::exists(folderPath)) {
        std::filesystem::remove_all(folderPath);
        std::cout << "LOG: Old zip output folder: \"" << folderPath << "\" was removed" << std::endl;
    }

    std::filesystem::create_directory(folderPath);

    int status;

    // Iterate over all the zip file entries
    for (int i = 0; i < numEntries; i++) {
        const char* fileName = zip_get_name(zipFile, i, ZIP_FL_ENC_GUESS);
        if (fileName == nullptr) {
            const char* strerror = zip_strerror(zipFile);
            std::cout << "ERROR: Failed to get file name of entry: " << i << " from zip file: \"" << zipPath << "\"" << std::endl;
            std::cout << "\t" << strerror << std::endl;
            zip_discard(zipFile);

            return false;
        }

        zip_file_t* fileEntry = zip_fopen(zipFile, fileName, ZIP_FL_COMPRESSED);
        if (fileEntry == nullptr) {
            const char* strerror = zip_strerror(zipFile);
            std::cout << "ERROR: Failed to read file entry: " << fileName << " from zip file: \"" << zipPath << "\"" << std::endl;
            std::cout << "\t" << strerror << std::endl;
            zip_discard(zipFile);

            return false;
        }

        const std::string entryNameStr(fileName);

        // Handle folder entries
        if (entryNameStr.ends_with("/") || entryNameStr.ends_with("\\")) {
            std::filesystem::create_directory(folderPath + entryNameStr);
        }
        // Handle file entries
        else {
            status = zip_file_is_seekable(fileEntry);
            if (status == 0) {
                std::cout << "ERROR: The file: \"" << fileName << "\" is not seekable, cannot determine file size" << std::endl;
                zip_discard(zipFile);

                return false;
            } else if (status == -1) {
                const char* strerror = zip_strerror(zipFile);
                std::cout << "ERROR: The file: \"" << fileName << "\" has an invalid configuration" << std::endl;
                std::cout << "\t" << strerror << std::endl;
                zip_discard(zipFile);

                return false;
            }

            // Seek to the end of the file
            status = (int)zip_fseek(fileEntry, 0, SEEK_END);
            if (status < 0) {
                const char* strerror = zip_strerror(zipFile);
                std::cout << "ERROR: The file: \"" << fileName << "\" could not be fseek-ed, cannot determine file size" << std::endl;
                std::cout << "\t" << strerror << std::endl;
                zip_discard(zipFile);

                return false;
            }

            // Get the cursor position
            int fileSize = zip_ftell(fileEntry);
            if (fileSize < 0) {
                const char* strerror = zip_strerror(zipFile);
                std::cout << "ERROR: The file: \"" << fileName << "\" could not be ftell-ed, cannot determine file size" << std::endl;
                std::cout << "\t" << strerror << std::endl;

                zip_discard(zipFile);

                return false;
            }

            // Seek to the begginning of the file
            status = (int)zip_fseek(fileEntry, 0, SEEK_SET);
            if (status < 0) {
                const char* strerror = zip_strerror(zipFile);
                std::cout << "ERROR: The file: \"" << fileName << "\" could not be fseek-ed, cannot determine file size" << std::endl;
                std::cout << "\t" << strerror << std::endl;
                zip_discard(zipFile);

                return false;
            }

            // Set up the output stream for the new zip file
            std::ofstream outputStream(folderPath + entryNameStr);
            if (outputStream.fail()) {
                std::cout << "ERROR: Failed to create new file for: \"" << fileName << "\"" << std::endl;
                zip_discard(zipFile);

                return false;
            }

            // Set up the buffer for the zip file
            char* buf = new char[fileSize];
            zip_int64_t readStatus = zip_fread(fileEntry, buf, fileSize);
            if (readStatus != fileSize) {
                const char* strerror = zip_strerror(zipFile);
                std::cout << "ERROR: Failed to read data from the file: \"" << fileName << "\"" << std::endl;
                std::cout << "\t" << strerror << std::endl;

                zip_discard(zipFile);
                outputStream.close();
                delete[] buf;

                return false;
            }

            // Write the bytes to the new file
            outputStream.write(buf, fileSize);

            outputStream.close();
            delete[] buf;
        }
    }

    return true;
}