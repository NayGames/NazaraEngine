#include <Engine/Shader/ShaderUtils.hpp>
#include <Nazara/Core/File.hpp>
#include <Nazara/Core/StringExt.hpp>
#include <Nazara/Shader/ShaderBuilder.hpp>
#include <Nazara/Shader/ShaderLangParser.hpp>
#include <catch2/catch.hpp>
#include <cctype>

TEST_CASE("structure member access", "[Shader]")
{
	SECTION("Nested member loading")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct innerStruct
{
	field: vec3[f32]
}

struct outerStruct
{
	s: innerStruct
}

external
{
	[set(0), binding(0)] ubo: uniform[outerStruct]
}
)";

		Nz::ShaderAst::ModulePtr shaderModule = Nz::ShaderLang::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		SECTION("Nested AccessMember")
		{
			auto ubo = Nz::ShaderBuilder::Identifier("ubo");
			auto firstAccess = Nz::ShaderBuilder::AccessMember(std::move(ubo), { "s" });
			auto secondAccess = Nz::ShaderBuilder::AccessMember(std::move(firstAccess), { "field" });

			auto swizzle = Nz::ShaderBuilder::Swizzle(std::move(secondAccess), { 2u });
			auto varDecl = Nz::ShaderBuilder::DeclareVariable("result", Nz::ShaderAst::ExpressionType{ Nz::ShaderAst::PrimitiveType::Float32 }, std::move(swizzle));

			shaderModule->rootNode->statements.push_back(Nz::ShaderBuilder::DeclareFunction(Nz::ShaderStageType::Vertex, "main", std::move(varDecl)));

			ExpectGLSL(*shaderModule, R"(
void main()
{
	float result = ubo.s.field.z;
}
)");

			ExpectNZSL(*shaderModule, R"(
[entry(vert)]
fn main()
{
	let result: f32 = ubo.s.field.z;
}
)");

			ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpAccessChain
OpLoad
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");
		}

		SECTION("AccessMember with multiples fields")
		{
			auto ubo = Nz::ShaderBuilder::Identifier("ubo");
			auto access = Nz::ShaderBuilder::AccessMember(std::move(ubo), { "s", "field" });

			auto swizzle = Nz::ShaderBuilder::Swizzle(std::move(access), { 2u });
			auto varDecl = Nz::ShaderBuilder::DeclareVariable("result", Nz::ShaderAst::ExpressionType{ Nz::ShaderAst::PrimitiveType::Float32 }, std::move(swizzle));

			shaderModule->rootNode->statements.push_back(Nz::ShaderBuilder::DeclareFunction(Nz::ShaderStageType::Vertex, "main", std::move(varDecl)));

			ExpectGLSL(*shaderModule, R"(
void main()
{
	float result = ubo.s.field.z;
}
)");

			ExpectNZSL(*shaderModule, R"(
[entry(vert)]
fn main()
{
	let result: f32 = ubo.s.field.z;
}
)");

			ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpAccessChain
OpLoad
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");
		}
	}
}