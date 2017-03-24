//
//  Copyright (c) 2014-2017 CASM Organization
//  All rights reserved.
//
//  Developed by: Philipp Paulweber
//                Emmanuel Pescosta
//                Florian Hahn
//                https://github.com/casm-lang/casmi
//
//  This file is part of casmi.
//
//  casmi is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  casmi is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with casmi. If not, see <http://www.gnu.org/licenses/>.
//

#include "license.h"
#include "version.h"

#include "libpass.h"
#include "libstdhl.h"

#include "libcasm-fe.h"
#include "libcasm-ir.h"
// #include "libcasm-rt.h"
#include "libcasm-tc.h"

/**
    @brief TODO

    TODO
*/

#define DESCRIPTION "Corinthian Abstract State Machine (CASM) Interpreter\n"

int main( int argc, const char* argv[] )
{
    const char* file_name = 0;
    u1 flag_dump_updates = false;

    const auto source
        = libstdhl::make< libstdhl::Log::Source >( argv[ 0 ], DESCRIPTION );

    libstdhl::Log::defaultSource( source );

    libstdhl::Args options( argc, argv, libstdhl::Args::DEFAULT,
        [&file_name, &options]( const char* arg ) {
            static int cnt = 0;
            cnt++;

            if( cnt > 1 )
            {
                options.m_error( 1, "too many file names passed" );
            }

            file_name = arg;
        } );

    options.add( 't', "test-case-profile", libstdhl::Args::NONE,
        "Display the unique test profile identifier and exit.",
        [&options]( const char* option ) {
            printf( "%s\n",
                libcasm_tc::Profile::get( libcasm_tc::Profile::INTERPRETER ) );
            exit( 0 );
        } );

    options.add( 'h', "help", libstdhl::Args::NONE,
        "Display the program usage and synopsis and exit.",
        [&options]( const char* option ) {
            fprintf( stderr, DESCRIPTION
                "\n"
                "usage: %s [options] <file>\n"
                "\n"
                "options:\n",
                options.programName() );

            options.m_usage();

            exit( 0 );
        } );

    options.add( 'v', "version", libstdhl::Args::NONE,
        "Display interpreter version information",
        [&options]( const char* option ) {
            fprintf( stderr, DESCRIPTION
                "\n"
                "%s: version: %s [ %s %s ]\n"
                "\n"
                "%s",
                options.programName(), VERSION, __DATE__, __TIME__, LICENSE );

            exit( 0 );
        } );

    options.add( 'd', "dump-updates", libstdhl::Args::NONE,
        "TBD DESCRIPTION dump updates (updateset)",
        [&flag_dump_updates](
            const char* option ) { flag_dump_updates = true; } );

    for( auto& p : libpass::PassRegistry::registeredPasses() )
    {
        // PassId    id = p.first;
        libpass::PassInfo& pi = *p.second;

        if( pi.argChar() == 0 && pi.argString() == 0 )
        {
            // internal pass, do not register a cmd line flag
            continue;
        }

        options.add( pi.argChar(), pi.argString(), libstdhl::Args::NONE,
            pi.description(), pi.argAction() );
    }

    options.parse();

    if( !file_name )
    {
        options.m_error( 1, "no input file provided" );
    }

    // TODO: FIXME: the following code should be implemented in the PassManager
    // structure
    // to allow dynamic and possible pass calls etc.

    libpass::PassResult x;

    auto load_file_pass = std::static_pointer_cast< libpass::LoadFilePass >(
        libpass::PassRegistry::passInfo< libpass::LoadFilePass >()
            .constructPass() );
    load_file_pass->setFilename( file_name );
    if( not load_file_pass->run( x ) )
    {
        return -1;
    }

    auto src_to_ast
        = libpass::PassRegistry::passInfo< libcasm_fe::SourceToAstPass >();

    auto ast_attribution
        = libpass::PassRegistry::passInfo< libcasm_fe::AttributionPass >();

    /*auto ast_check
        = libpass::PassRegistry::passInfo< libcasm_fe::TypeCheckPass >();*/

    auto ast_dump
        = libpass::PassRegistry::passInfo< libcasm_fe::AstDumpDotPass >();

    /*auto ast_exec_sym = libpass::PassRegistry::
        passInfo< libcasm_fe::SymbolicExecutionPass >();

    auto ast_exec_num
        = libpass::PassRegistry::passInfo< libcasm_fe::NumericExecutionPass >();

    auto ast_to_ir
        = libpass::PassRegistry::passInfo< libcasm_fe::AstToCasmIRPass >();*/

    auto ir_check
        = libpass::PassRegistry::passInfo< libcasm_ir::ConsistencyCheckPass >();

    auto ir_dbg
        = libpass::PassRegistry::passInfo< libcasm_ir::IRDumpDebugPass >();

    auto ir_src
        = libpass::PassRegistry::passInfo< libcasm_ir::IRDumpSourcePass >();

    auto ir_dot
        = libpass::PassRegistry::passInfo< libcasm_ir::IRDumpDotPass >();

    // auto ir_cf
    //     = libpass::PassRegistry::passInfo< libcasm_ir::ConstantFoldingPass >();

    if( src_to_ast.constructPass()->run( x ) )
    {
        if( src_to_ast.isArgSelected() )
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }

    if( ast_attribution.constructPass()->run( x ) )
    {
        if( ast_attribution.isArgSelected() )
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }

    /*if( ast_check.constructPass()->run( x ) )
    {
        if( ast_check.isArgSelected() )
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }*/

    if( ast_dump.isArgSelected() )
    {
        if( not ast_dump.constructPass()->run( x ) )
        {
            return -1;
        }
        return 0; // TODO remove me
    }

    /*if( ast_exec_sym.isArgSelected() )
    {
        return ast_exec_sym.constructPass()->run( x ) ? 0 : -1;
    }

    auto ast_exec_num_pass
        = std::static_pointer_cast< libcasm_fe::NumericExecutionPass >(
            ast_exec_num.constructPass() );
    ast_exec_num_pass->setDumpUpdates( flag_dump_updates );

    if( ast_exec_num.isArgSelected() )
    {
        return ast_exec_num_pass->run( x ) ? 0 : -1;
    }

    if( not ast_to_ir.isArgSelected() and not ir_check.isArgSelected()
        and not ir_dbg.isArgSelected()
        and not ir_src.isArgSelected()
        and not ir_dot.isArgSelected()
        // and not ir_cf.isArgSelected()
        )
    {
        libstdhl::Log::info( "no command provided, using '--ast-exec-num'" );
        return ast_exec_num_pass->run( x ) ? 0 : -1;
    }

    if( ast_to_ir.constructPass()->run( x ) )
    {
        if( ast_to_ir.isArgSelected() )
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }*/

    if( ir_check.constructPass()->run( x ) )
    {
        if( ir_check.isArgSelected() )
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }

    // if( ir_cf.isArgSelected() )
    // {
    //     if( not ir_cf.constructPass()->run( x ) )
    //     {
    //         return -1;
    //     }
    // }

    if( ir_dbg.isArgSelected() )
    {
        return ir_dbg.constructPass()->run( x ) ? 0 : -1;
    }

    if( ir_src.isArgSelected() )
    {
        return ir_src.constructPass()->run( x ) ? 0 : -1;
    }

    if( ir_dot.isArgSelected() )
    {
        return ir_dot.constructPass()->run( x ) ? 0 : -1;
    }

    libstdhl::Log::error( "no valid command provided!" );
    return -1;
}

//
//  Local variables:
//  mode: c++
//  indent-tabs-mode: nil
//  c-basic-offset: 4
//  tab-width: 4
//  End:
//  vim:noexpandtab:sw=4:ts=4:
//
