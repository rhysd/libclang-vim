#if !defined LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED
#define      LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

#include <cctype>
#include <string>
#include <stack>
#include <set>

#include "helpers.hpp"
#include "stringizers.hpp"

namespace libclang_vim {

namespace detail {

inline bool is_auto_type(std::string const& type_name)
{
    for ( auto pos = type_name.find("auto")
        ; pos != std::string::npos
        ; pos = type_name.find("auto", pos+1)) {

        if (pos != 0) {
            if (std::isalnum(type_name[pos-1]) || type_name[pos-1] == '_') {
                continue;
            }
        }

        if (pos + 3/*pos of 'o'*/ < type_name.size()-1) {
            if (std::isalnum(type_name[pos+3+1]) || type_name[pos+3+1] == '_') {
                continue;
            }
        }

        return true;
    }

    return false;
}

inline CXChildVisitResult unexposed_type_deducer(CXCursor cursor, CXCursor, CXClientData data)
{
    auto const type = clang_getCursorType(cursor);
    cxstring_ptr type_name = clang_getTypeSpelling(type);
    if (type.kind == CXType_Invalid || is_auto_type(to_c_str(type_name))) {
        clang_visitChildren(cursor, unexposed_type_deducer, data);
        return CXChildVisit_Continue;
    } else {
        *(reinterpret_cast<CXType *>(data)) = type;
        return CXChildVisit_Break;
    }
}

inline CXType deduce_type_at_cursor(CXCursor const& cursor)
{
    auto const type = clang_getCursorType(cursor);
    cxstring_ptr type_name = clang_getTypeSpelling(type);
    if (type.kind == CXType_Invalid || is_auto_type(to_c_str(type_name))) {
        CXType deduced_type;
        deduced_type.kind = CXType_Invalid;
        clang_visitChildren(cursor, unexposed_type_deducer, &deduced_type);
        return deduced_type;
    } else {
        return type;
    }
}

} // namespace detail

inline char const* deduce_var_decl_type(const location_tuple& location_info)
{
    return at_specific_location(
                location_info,
                [](CXCursor const& cursor)
                    -> std::string
                {
                    CXCursor const var_decl_cursor = search_kind(cursor, [](CXCursorKind const& kind){ return kind == CXCursor_VarDecl; });
                    if (clang_Cursor_isNull(var_decl_cursor)) {
                        return "{}";
                    }

                    CXType const var_type = detail::deduce_type_at_cursor(var_decl_cursor);
                    if (var_type.kind == CXType_Invalid) {
                        return "{}";
                    }

                    std::string result;
                    result += stringize_type(var_type);
                    result += "'canonical':{" + stringize_type(clang_getCanonicalType(var_type)) + "},";
                    return "{" + result + "}";
                }
            );
}

const char* deduce_func_return_type(const location_tuple& location_info);

const char* deduce_func_or_var_decl(const location_tuple& location_info);

/// Get type at specific location with auto-deduction described above.
const char* deduce_type_at(const location_tuple& location_info);

/// Wrapper around clang_getCursorSpelling().
const char* get_current_function_at(const location_tuple& location_info);

/// Wrapper around clang_Cursor_getBriefCommentText().
const char* get_comment_at(const location_tuple& location_info);

/// Get location of declaration referenced by location_info.
const char* get_deduced_declaration_at(const location_tuple& location_info);

/// Wrapper around clang_getIncludedFile().
const char* get_include_at(const location_tuple& location_info);

/// Wrapper around clang_codeCompleteAt().
const char* get_completion_at(const location_tuple& location_info);

/// Wrapper around clang_CompilationDatabase_getCompileCommands().
const char* get_compile_commands(const std::string& file);

/// Wrapper around clang_getDiagnostic().
const char* get_diagnostics(const std::pair<std::string, args_type>& file_and_args);

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
