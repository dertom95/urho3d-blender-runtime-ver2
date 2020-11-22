#pragma once

#include<Urho3D/Container/Str.h>
#include<Urho3D/Container/Vector.h>
#include<Urho3D/Core/Object.h>
#include<Urho3D/IO/File.h>
#include<Urho3D/IO/FileSystem.h>
using namespace Urho3D;

class PackageTool : public Object {
    URHO3D_OBJECT(PackageTool,Object)
public:
    PackageTool(Context* ctx_);
    void WritePackageFile(const String& fileName, const String& rootDir);
    void Run(const Vector<String>& arguments);
private:
    void ProcessFile(const String& fileName, const String& rootDir);
    void WriteHeader(File& dest);

    static const unsigned COMPRESSED_BLOCK_SIZE = 32768;

    struct FileEntry
    {
        String name_;
        unsigned offset_{};
        unsigned size_{};
        unsigned checksum_{};
    };


    String basePath_;
    Vector<FileEntry> entries_;
    unsigned checksum_ = 0;
    bool compress_ = false;
    bool quiet_ = false;
    unsigned blockSize_ = COMPRESSED_BLOCK_SIZE;

    Vector<String> ignoreExtensions_;
};

