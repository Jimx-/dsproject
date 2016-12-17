#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <stdexcept>

enum ExceptionCode {
    E_FILE_NOT_FOUND = 1,
    E_SYSTEM_ERROR = 2,
    E_RESOURCE_ERROR = 3,
    E_RENDER_ENGINE_ERROR = 4,
    E_INVALID_STATE = 5,
    E_INVALID_PARAM = 6,
};

class Exception : public std::exception {
public:
    Exception(ExceptionCode code, const std::string& source, const std::string& descr, const char* type, const char* file, int line);
    ~Exception() noexcept { }

    const std::string& make_description() const;

    virtual const char* what() const noexcept { return make_description().c_str(); }

protected:
    ExceptionCode code;
    std::string source;
    std::string descr;
    std::string type;
    std::string file;
    int line;

    mutable std::string full_descr;
};

#define DEF_EXCEPTION(name) \
    class name : public Exception { \
    public: \
        name (ExceptionCode code, const std::string& source, const std::string& descr, const char* file, int line) : \
            Exception(code, source, descr, #name, file, line) { } \
    }

DEF_EXCEPTION(FileNotFoundException);
DEF_EXCEPTION(SystemError);
DEF_EXCEPTION(ResourceError);
DEF_EXCEPTION(RenderEngineError);
DEF_EXCEPTION(InvalidStateException);
DEF_EXCEPTION(InvalidParameterException);

template <int c>
struct ExceptionCodeType {
    enum { code = c };
};

class ExceptionFactory {
public:
    static FileNotFoundException create(ExceptionCodeType<ExceptionCode::E_FILE_NOT_FOUND> code, const std::string& source,
                                 const std::string& descr, const char* file, int line) {
        return FileNotFoundException((ExceptionCode)code.code, source, descr, file, line);
    }
    static SystemError create(ExceptionCodeType<ExceptionCode::E_SYSTEM_ERROR> code, const std::string& source,
                                        const std::string& descr, const char* file, int line) {
        return SystemError((ExceptionCode)code.code, source, descr, file, line);
    }
    static ResourceError create(ExceptionCodeType<ExceptionCode::E_RESOURCE_ERROR> code, const std::string& source,
                              const std::string& descr, const char* file, int line) {
        return ResourceError((ExceptionCode)code.code, source, descr, file, line);
    }
    static RenderEngineError create(ExceptionCodeType<ExceptionCode::E_RENDER_ENGINE_ERROR> code, const std::string& source,
                                           const std::string& descr, const char* file, int line) {
        return RenderEngineError((ExceptionCode)code.code, source, descr, file, line);
    }
    static InvalidStateException create(ExceptionCodeType<ExceptionCode::E_INVALID_STATE> code, const std::string& source,
                                    const std::string& descr, const char* file, int line) {
        return InvalidStateException((ExceptionCode)code.code, source, descr, file, line);
    }

    static InvalidParameterException create(ExceptionCodeType<ExceptionCode::E_INVALID_PARAM> code, const std::string& source,
                                        const std::string& descr, const char* file, int line) {
        return InvalidParameterException((ExceptionCode)code.code, source, descr, file, line);
    }
};

#define THROW_EXCEPT(code, src, descr) throw ExceptionFactory::create(ExceptionCodeType<code>(), src, descr, __FILE__, \
                             __LINE__)

#endif
