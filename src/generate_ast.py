import sys

def define_ast_utils(writer, base_name, class_name, field_list):
    # Pointer make wrapper
    writer.write("inline std::shared_ptr<%s> make_%s(%s) {\n" % (base_name, class_name.lower(), field_list))
    writer.write("    return std::make_shared<%s>(" % (class_name))

    fields = field_list.split(", ")
    for idx, field in enumerate(fields):
        name = field.split(" ")[1]
        writer.write("%s" % name)

        if idx != len(fields) - 1:
            writer.write(", ")

    writer.write(");\n")
    writer.write("}\n\n")

def define_visitor(writer, base_name, types):
    writer.write("class Visitor {\n")
    writer.write("public:\n")
    for t in types:
        type_name = t.split("|")[0].strip()
        writer.write(
            "    virtual std::any visit%s%s(%s* %s) = 0;\n" % ( type_name,
                                                                base_name,
                                                                type_name,
                                                                base_name.lower()))

    writer.write("};\n\n")

def define_type(writer, base_name, class_name, field_list):
    writer.write("struct %s : public %s {\n" % (class_name, base_name))

    # Constructor
    writer.write("    %s(%s) :\n" % (class_name, field_list))

    # Assignt parameters in field
    fields = field_list.split(", ")
    for idx, field in enumerate(fields):
        name = field.split(" ")[1]
        writer.write("        m_%s(%s)" % (name, name))

        if idx != len(fields) - 1:
            writer.write(",")
        
        writer.write("\n")

    writer.write("    {}\n\n")

    # Destructor
    writer.write("    virtual ~%s() = default;\n" % (class_name))

    # Visitor
    writer.write("\n")
    writer.write("    std::any accept(Visitor* visitor) {\n")
    writer.write("        return visitor->visit%s%s(this);\n" % (class_name, base_name))
    writer.write("    }\n\n")

    # Fields
    for field in fields:
        field_type = field.split(" ")[0]
        name = field.split(" ")[1]
        writer.write("    const %s m_%s;\n" % (field_type, name))

    writer.write("};\n\n")

    define_ast_utils(writer, base_name, class_name, field_list)


def define_ast(output_dir, base_name, types, dependencies):
    path = output_dir + "/nex_" + base_name.lower() + ".hpp"

    writer = open(path, "w", encoding="UTF-8")

    writer.write("#ifndef NEX_%s_HPP_\n" % base_name.upper())
    writer.write("#define NEX_%s_HPP_\n" % base_name.upper())
    writer.write("\n")

    for dep in dependencies:
        writer.write("#include \"%s.hpp\"\n" % dep)

    writer.write("#include <memory>\n")
    writer.write("#include <vector>\n\n")

    writer.write("namespace nex::ast::%s {\n" % base_name.lower())

    # Forward declarations
    for type in types:
        name = type.split("|")[0].strip()
        writer.write("struct %s;\n" % name)
    writer.write("\n")

    define_visitor(writer, base_name, types)

    writer.write("struct %s {\n" % base_name)
    writer.write("    virtual std::any accept(Visitor* visitor) = 0;\n")
    writer.write("};\n\n")

    for type in types:
        name = type.split("|")[0].strip()
        fields = type.split("|")[1].strip()
        define_type(writer, base_name, name, fields)

    writer.write("}\n")

    writer.write("#endif\n")

    writer.close()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: generate_ast.py <output directory>", file=sys.stderr)
        exit(1)
    else:
        output_dir = sys.argv[1]
        define_ast(output_dir, "Expr", [
            "Assign     | Token name, std::shared_ptr<Expr> value",
            "Binary     | std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right",
            "Call       | std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>> arguments",
            "Get        | std::shared_ptr<Expr> object, Token name",
            "Set        | std::shared_ptr<Expr> object, Token name, std::shared_ptr<Expr> value",
            "Super      | Token keyword, Token method",
            "This       | Token keyword",
            "Grouping   | std::shared_ptr<Expr> expression",
            "Literal    | std::any value",
            "Logical    | std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right",
            "Unary      | Token op, std::shared_ptr<Expr> right",
            "Comma      | std::vector<std::shared_ptr<Expr>> exprs, std::shared_ptr<Expr> last",
            "Variable   | Token name",
            "Input      | void* e",
        ], ["token"])

        define_ast(output_dir, "Stmt", [
            "Block      | std::vector<std::shared_ptr<Stmt>> statements",
            "Class      | Token name, std::shared_ptr<expr::Variable> superclass, std::vector<std::shared_ptr<Function>> methods, std::vector<std::shared_ptr<Let>> fields",
            "Expression | std::shared_ptr<expr::Expr> e",
            "Function   | Token name, std::vector<Token> params, std::vector<std::shared_ptr<Stmt>> body",
            "If         | std::shared_ptr<expr::Expr> cond, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch",
            "Print      | std::shared_ptr<expr::Expr> e",
            "Return     | Token keyword, std::shared_ptr<expr::Expr> value",
            "Let        | Token name, std::shared_ptr<expr::Expr> init",
            "While      | std::shared_ptr<expr::Expr> cond, std::shared_ptr<Stmt> body",
        ], ["token", "expr",])
