# Generated by jeweler
# DO NOT EDIT THIS FILE DIRECTLY
# Instead, edit Jeweler::Tasks in Rakefile, and run the gemspec command
# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = %q{dub}
  s.version = "0.6.2"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = ["Gaspard Bucher"]
  s.date = %q{2010-03-12}
  s.description = %q{This is a tool to ease the creation of scripting language bindings for a C++ library.
    It is currently developed to crete the OpenCV bindings for Lua in Rubyk (http://rubyk.org). The generator uses the xml output from Doxygen to avoid parsing C++ code by itself.}
  s.email = %q{gaspard@teti.ch}
  s.extra_rdoc_files = [
    "LICENSE",
     "README.rdoc"
  ]
  s.files = [
    ".gitignore",
     "History.txt",
     "LICENSE",
     "README.rdoc",
     "Rakefile",
     "dub.gemspec",
     "lib/dub.rb",
     "lib/dub/argument.rb",
     "lib/dub/entities_unescape.rb",
     "lib/dub/function.rb",
     "lib/dub/function_group.rb",
     "lib/dub/generator.rb",
     "lib/dub/group.rb",
     "lib/dub/klass.rb",
     "lib/dub/lua.rb",
     "lib/dub/lua/class.cpp.erb",
     "lib/dub/lua/class_gen.rb",
     "lib/dub/lua/function.cpp.erb",
     "lib/dub/lua/function_gen.rb",
     "lib/dub/lua/group.cpp.erb",
     "lib/dub/lua/lua_cpp_helper.h",
     "lib/dub/lua/namespace.cpp.erb",
     "lib/dub/lua/namespace_gen.rb",
     "lib/dub/member_extraction.rb",
     "lib/dub/namespace.rb",
     "lib/dub/parser.rb",
     "lib/dub/templates/lua_template.erb",
     "lib/dub/version.rb",
     "test/argument_test.rb",
     "test/fixtures/app/CMakeLists.txt",
     "test/fixtures/app/Doxyfile",
     "test/fixtures/app/bindings/all_lua.cpp",
     "test/fixtures/app/include/matrix.h",
     "test/fixtures/app/make_lua_bindings.rb",
     "test/fixtures/app/vendor/lua/CMakeLists.txt",
     "test/fixtures/app/vendor/lua/COPYRIGHT",
     "test/fixtures/app/vendor/lua/HISTORY",
     "test/fixtures/app/vendor/lua/INSTALL",
     "test/fixtures/app/vendor/lua/Makefile",
     "test/fixtures/app/vendor/lua/README",
     "test/fixtures/app/vendor/lua/lapi.c",
     "test/fixtures/app/vendor/lua/lapi.h",
     "test/fixtures/app/vendor/lua/lauxlib.c",
     "test/fixtures/app/vendor/lua/lauxlib.h",
     "test/fixtures/app/vendor/lua/lbaselib.c",
     "test/fixtures/app/vendor/lua/lcode.c",
     "test/fixtures/app/vendor/lua/lcode.h",
     "test/fixtures/app/vendor/lua/ldblib.c",
     "test/fixtures/app/vendor/lua/ldebug.c",
     "test/fixtures/app/vendor/lua/ldebug.h",
     "test/fixtures/app/vendor/lua/ldo.c",
     "test/fixtures/app/vendor/lua/ldo.h",
     "test/fixtures/app/vendor/lua/ldump.c",
     "test/fixtures/app/vendor/lua/lfunc.c",
     "test/fixtures/app/vendor/lua/lfunc.h",
     "test/fixtures/app/vendor/lua/lgc.c",
     "test/fixtures/app/vendor/lua/lgc.h",
     "test/fixtures/app/vendor/lua/liblua.a",
     "test/fixtures/app/vendor/lua/linit.c",
     "test/fixtures/app/vendor/lua/liolib.c",
     "test/fixtures/app/vendor/lua/llex.c",
     "test/fixtures/app/vendor/lua/llex.h",
     "test/fixtures/app/vendor/lua/llimits.h",
     "test/fixtures/app/vendor/lua/lmathlib.c",
     "test/fixtures/app/vendor/lua/lmem.c",
     "test/fixtures/app/vendor/lua/lmem.h",
     "test/fixtures/app/vendor/lua/loadlib.c",
     "test/fixtures/app/vendor/lua/lobject.c",
     "test/fixtures/app/vendor/lua/lobject.h",
     "test/fixtures/app/vendor/lua/lopcodes.c",
     "test/fixtures/app/vendor/lua/lopcodes.h",
     "test/fixtures/app/vendor/lua/loslib.c",
     "test/fixtures/app/vendor/lua/lparser.c",
     "test/fixtures/app/vendor/lua/lparser.h",
     "test/fixtures/app/vendor/lua/lstate.c",
     "test/fixtures/app/vendor/lua/lstate.h",
     "test/fixtures/app/vendor/lua/lstring.c",
     "test/fixtures/app/vendor/lua/lstring.h",
     "test/fixtures/app/vendor/lua/lstrlib.c",
     "test/fixtures/app/vendor/lua/ltable.c",
     "test/fixtures/app/vendor/lua/ltable.h",
     "test/fixtures/app/vendor/lua/ltablib.c",
     "test/fixtures/app/vendor/lua/ltm.c",
     "test/fixtures/app/vendor/lua/ltm.h",
     "test/fixtures/app/vendor/lua/lua.c",
     "test/fixtures/app/vendor/lua/lua.h",
     "test/fixtures/app/vendor/lua/lua_dub_helper.h",
     "test/fixtures/app/vendor/lua/luac",
     "test/fixtures/app/vendor/lua/luac.c",
     "test/fixtures/app/vendor/lua/luaconf.h",
     "test/fixtures/app/vendor/lua/lualib.h",
     "test/fixtures/app/vendor/lua/lundump.c",
     "test/fixtures/app/vendor/lua/lundump.h",
     "test/fixtures/app/vendor/lua/lvm.c",
     "test/fixtures/app/vendor/lua/lvm.h",
     "test/fixtures/app/vendor/lua/lzio.c",
     "test/fixtures/app/vendor/lua/lzio.h",
     "test/fixtures/app/vendor/lua/matrix.h",
     "test/fixtures/app/vendor/lua/print.c",
     "test/fixtures/app/vendor/lua/test/README",
     "test/fixtures/app/vendor/lua/test/bisect.lua",
     "test/fixtures/app/vendor/lua/test/cf.lua",
     "test/fixtures/app/vendor/lua/test/echo.lua",
     "test/fixtures/app/vendor/lua/test/env.lua",
     "test/fixtures/app/vendor/lua/test/factorial.lua",
     "test/fixtures/app/vendor/lua/test/fib.lua",
     "test/fixtures/app/vendor/lua/test/fibfor.lua",
     "test/fixtures/app/vendor/lua/test/globals.lua",
     "test/fixtures/app/vendor/lua/test/hello.lua",
     "test/fixtures/app/vendor/lua/test/life.lua",
     "test/fixtures/app/vendor/lua/test/luac.lua",
     "test/fixtures/app/vendor/lua/test/printf.lua",
     "test/fixtures/app/vendor/lua/test/readonly.lua",
     "test/fixtures/app/vendor/lua/test/sieve.lua",
     "test/fixtures/app/vendor/lua/test/sort.lua",
     "test/fixtures/app/vendor/lua/test/table.lua",
     "test/fixtures/app/vendor/lua/test/trace-calls.lua",
     "test/fixtures/app/vendor/lua/test/trace-globals.lua",
     "test/fixtures/app/vendor/lua/test/xd.lua",
     "test/fixtures/app/xml/classdub_1_1_matrix.xml",
     "test/fixtures/app/xml/classdub_1_1_t_mat.xml",
     "test/fixtures/app/xml/combine.xslt",
     "test/fixtures/app/xml/compound.xsd",
     "test/fixtures/app/xml/dir_53661a2bdeb1d55e60581a7e15deb763.xml",
     "test/fixtures/app/xml/index.xml",
     "test/fixtures/app/xml/index.xsd",
     "test/fixtures/app/xml/matrix_8h.xml",
     "test/fixtures/app/xml/namespacedub.xml",
     "test/fixtures/classcv_1_1_mat.xml",
     "test/fixtures/classcv_1_1_point__.xml",
     "test/fixtures/classcv_1_1_scalar__.xml",
     "test/fixtures/classcv_1_1_size__.xml",
     "test/fixtures/dummy_class.cpp.erb",
     "test/fixtures/dummy_function.cpp.erb",
     "test/fixtures/group___magic_type.xml",
     "test/fixtures/namespacecv.xml",
     "test/function_group_test.rb",
     "test/function_test.rb",
     "test/group_test.rb",
     "test/helper.rb",
     "test/klass_test.rb",
     "test/lua_function_gen_test.rb",
     "test/namespace_test.rb",
     "test/parser_test.rb"
  ]
  s.homepage = %q{http://github.com/ruby/dub}
  s.rdoc_options = ["--charset=UTF-8"]
  s.require_paths = ["lib"]
  s.rubygems_version = %q{1.3.5}
  s.summary = %q{A tool to ease the creation of scripting language bindings for a C++ library.}
  s.test_files = [
    "test/argument_test.rb",
     "test/fixtures/app/make_lua_bindings.rb",
     "test/function_group_test.rb",
     "test/function_test.rb",
     "test/group_test.rb",
     "test/helper.rb",
     "test/klass_test.rb",
     "test/lua_function_gen_test.rb",
     "test/namespace_test.rb",
     "test/parser_test.rb"
  ]

  if s.respond_to? :specification_version then
    current_version = Gem::Specification::CURRENT_SPECIFICATION_VERSION
    s.specification_version = 3

    if Gem::Version.new(Gem::RubyGemsVersion) >= Gem::Version.new('1.2.0') then
      s.add_development_dependency(%q<shoulda>, [">= 0"])
    else
      s.add_dependency(%q<shoulda>, [">= 0"])
    end
  else
    s.add_dependency(%q<shoulda>, [">= 0"])
  end
end

