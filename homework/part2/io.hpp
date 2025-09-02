#pragma once

#include "profiler.hpp"

struct File {
    FILE* data;

#ifdef DEBUG
    cstr path;
    u64  modTime;
#endif

#ifdef DEBUG
    static Result<File> New(cstr _path, cstr _mode) : data{fopen(_path, _mode)}, path{_path}, modTime{0} {}
#else
    static Result<File> New(cstr _path, cstr _mode) {
        FILE* data = fopen(_path, _mode);

        if (data) {
            return Result<File>::Ok(File{.data = data});
        } else {
            return Result<File>::Err("Couldn't read file");
        }
    }
#endif

    ~File() {
        if (data) fclose(data);
    }

    operator FILE*() { return data; }
};

enum ReadFileError {
    RF_NONE,
    RF_COULDNT_OPEN,
    RF_COULDNT_READ,
};

Result<Array<u8>, ReadFileError> ReadEntireFile(cstr path) {
    using Result = Result<Array<u8>, ReadFileError>;
    PROFILE_FUNCTION();

    auto fr = File::New(path, "rb");
    if (!fr.ok) return {Result::Err(RF_COULDNT_OPEN)};

    auto f = fr.value;

    if (fseek(f, 0, SEEK_END) != 0) return {};

    u64 cap = ftell(f);
    if (cap == 0) return {}; // == ?
    PROFILE_ADD_BANDWIDTH(cap);

    auto result = Array<u8>::New(cap);
    if (!result.data) return Result::Err(RF_COULDNT_READ);
    result.len = result.cap;

    rewind(f);
    u64 rd = fread(result.data, 1, (u64)result.cap, f);
    if (rd != (u64)result.cap) return Result::Err(RF_COULDNT_READ);

    return Result::Ok(result);
}