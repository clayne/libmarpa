#!perl
# Copyright 2012 Jeffrey Kegler
# This file is part of Marpa::R2.  Marpa::R2 is free software: you can
# redistribute it and/or modify it under the terms of the GNU Lesser
# General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# Marpa::R2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser
# General Public License along with Marpa::R2.  If not, see
# http://www.gnu.org/licenses/.
# An ambiguous equation

use 5.010;
use strict;
use warnings;

use Test::More tests => 1;

use lib 'inc';
use Marpa::R2::Test;
use English qw( -no_match_vars );
use Fatal qw(open close);

BEGIN {
    use Marpa::R2;
}

## no critic (InputOutput::RequireBriefOpen)
open my $original_stdout, q{>&STDOUT};
## use critic

sub save_stdout {
    my $save;
    my $save_ref = \$save;
    close STDOUT;
    open STDOUT, q{>}, $save_ref;
    return $save_ref;
} ## end sub save_stdout

sub restore_stdout {
    close STDOUT;
    open STDOUT, q{>&}, $original_stdout;
    return 1;
}

# Marpa::R2::Display
# name: Null Value Example

sub L {
    shift;
    return 'L(' . ( join q{;}, map { $_ // '[ERROR!]' } @_ ) . ')';
}

sub R {
    shift;
    return 'R(' . ( join q{;}, map { $_ // '[ERROR!]' } @_ ) . ')';
}

sub S {
    shift;
    return 'S(' . ( join q{;}, map { $_ // '[ERROR!]' } @_ ) . ')';
}

my $grammar = Marpa::R2::Grammar->new(
    {   start   => 'S',
        actions => 'main',
        rules   => [
            [ 'S', [qw/L R/] ],
            [ 'L', [qw/A B X/] ],
            [ 'L', [] ],
            [ 'R', [qw/A B Y/] ],
            [ 'R', [] ],
            [ 'A', [] ],
            [ 'B', [] ],
            [ 'X', [] ],
            [ 'X', [qw/x/] ],
            [ 'Y', [] ],
            [ 'Y', [qw/y/] ],
        ],
        symbols => {
            L => { null_value => 'null L' },
            R => { null_value => 'null R' },
            A => { null_value => 'null A' },
            B => { null_value => 'null B' },
            X => { null_value => 'null X' },
            Y => { null_value => 'null Y' },
        },
    }
);

$grammar->precompute();

my $recce = Marpa::R2::Recognizer->new( { grammar => $grammar } );

$recce->read( 'x', 'x' );

# Marpa::R2::Display::End

## use critic

# Marpa::R2::Display
# name: Null Value Example Output
# start-after-line: END_OF_OUTPUT
# end-before-line: '^END_OF_OUTPUT$'

chomp( my $expected = <<'END_OF_OUTPUT');
S(L(null A;null B;x);null R)
END_OF_OUTPUT

# Marpa::R2::Display::End

my $value = $recce->value();
Marpa::R2::Test::is( ${$value}, $expected, 'Null example' );

1;    # In case used as "do" file

# Local Variables:
#   mode: cperl
#   cperl-indent-level: 4
#   fill-column: 100
# End:
# vim: expandtab shiftwidth=4:
