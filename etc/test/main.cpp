//
//  Copyright (C) 2014-2018 CASM Organization <https://casm-lang.org>
//  All rights reserved.
//
//  Developed by: Philipp Paulweber
//                Emmanuel Pescosta
//                Florian Hahn
//                <https://github.com/casm-lang/casmi>
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

#include "main.h"

#include <casmi/Version>

void casmi_main_dummy( void )
{
    const auto source = libstdhl::Memory::make< libstdhl::Log::Source >( "casmi", "CASMI" );
    libstdhl::Log::defaultSource( source );
}

TEST( casmi_main, empty )
{
    std::cout << casmi::REVTAG << "\n";
    std::cout << casmi::COMMIT << "\n";
    std::cout << casmi::BRANCH << "\n";
    std::cout << casmi::LICENSE << "\n";
    std::cout << casmi::NOTICE << "\n";
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
