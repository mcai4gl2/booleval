/*
 * Copyright (c) 2020, Marin Peko
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the
 *   distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <benchmark/benchmark.h>
#include <booleval/evaluator.hpp>
#include <string>
#include <sstream>

namespace
{

    template< typename T >
    class data_object
    {
    public:
        explicit data_object( T value ) : value_( std::move( value ) ) {}
        T value() const noexcept { return value_; }

    private:
        T value_{};
    };

    // Helper function to generate expression like: "field v1 or field v2 or ... or field vN"
    std::string generate_in_expression( const std::string& field_name, const std::vector<std::string>& values )
    {
        if ( values.empty() ) return "";

        std::ostringstream oss;
        oss << field_name << " " << values[0];
        
        for ( size_t i = 1; i < values.size(); ++i )
        {
            oss << " or " << field_name << " " << values[i];
        }
        
        return oss.str();
    }

} // namespace

// ============================================================================
// User's actual use case: Parse AND evaluate on every iteration
// ============================================================================

void ParseAndEvaluate_1Value( benchmark::State& state )
{
    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field", &data_object< std::string >::value )
        }
    };

    std::string expr = generate_in_expression( "field", { "value1" } );
    data_object< std::string > obj{ "value1" };

    for ( auto _ : state )
    {
        [[ maybe_unused ]] auto const success{ evaluator.expression( expr ) };
        [[ maybe_unused ]] auto const result{ evaluator.evaluate( obj ) };
        benchmark::DoNotOptimize( result );
        benchmark::ClobberMemory();
    }
}

BENCHMARK( ParseAndEvaluate_1Value );

void ParseAndEvaluate_5Values( benchmark::State& state )
{
    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field", &data_object< std::string >::value )
        }
    };

    std::string expr = generate_in_expression( "field", { "value1", "value2", "value3", "value4", "value5" } );
    data_object< std::string > obj{ "value3" };  // Match in middle

    for ( auto _ : state )
    {
        [[ maybe_unused ]] auto const success{ evaluator.expression( expr ) };
        [[ maybe_unused ]] auto const result{ evaluator.evaluate( obj ) };
        benchmark::DoNotOptimize( result );
        benchmark::ClobberMemory();
    }
}

BENCHMARK( ParseAndEvaluate_5Values );

void ParseAndEvaluate_10Values( benchmark::State& state )
{
    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field", &data_object< std::string >::value )
        }
    };

    std::string expr = generate_in_expression( "field", 
        { "value1", "value2", "value3", "value4", "value5",
          "value6", "value7", "value8", "value9", "value10" } );
    data_object< std::string > obj{ "value5" };  // Match in middle

    for ( auto _ : state )
    {
        [[ maybe_unused ]] auto const success{ evaluator.expression( expr ) };
        [[ maybe_unused ]] auto const result{ evaluator.evaluate( obj ) };
        benchmark::DoNotOptimize( result );
        benchmark::ClobberMemory();
    }
}

BENCHMARK( ParseAndEvaluate_10Values );

void ParseAndEvaluate_50Values( benchmark::State& state )
{
    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field", &data_object< std::string >::value )
        }
    };

    std::vector<std::string> values;
    for ( int i = 1; i <= 50; ++i )
    {
        values.push_back( "value" + std::to_string( i ) );
    }
    
    std::string expr = generate_in_expression( "field", values );
    data_object< std::string > obj{ "value25" };  // Match in middle

    for ( auto _ : state )
    {
        [[ maybe_unused ]] auto const success{ evaluator.expression( expr ) };
        [[ maybe_unused ]] auto const result{ evaluator.evaluate( obj ) };
        benchmark::DoNotOptimize( result );
        benchmark::ClobberMemory();
    }
}

BENCHMARK( ParseAndEvaluate_50Values );

// ============================================================================
// Benchmark scenario: Parse once, evaluate many times
// ============================================================================

void ParseOnceEvaluateMany_10Values( benchmark::State& state )
{
    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field", &data_object< std::string >::value )
        }
    };

    std::string expr = generate_in_expression( "field", 
        { "value1", "value2", "value3", "value4", "value5",
          "value6", "value7", "value8", "value9", "value10" } );
    
    [[ maybe_unused ]] auto const success{ evaluator.expression( expr ) };
    data_object< std::string > obj{ "value5" };  // Match in middle

    for ( auto _ : state )
    {
        [[ maybe_unused ]] auto const result{ evaluator.evaluate( obj ) };
        benchmark::DoNotOptimize( result );
        benchmark::ClobberMemory();
    }
}

BENCHMARK( ParseOnceEvaluateMany_10Values );

// ============================================================================
// Test short-circuit evaluation behavior
// ============================================================================

void ShortCircuitTest_FirstMatch_10Values( benchmark::State& state )
{
    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field", &data_object< std::string >::value )
        }
    };

    std::string expr = generate_in_expression( "field", 
        { "value1", "value2", "value3", "value4", "value5",
          "value6", "value7", "value8", "value9", "value10" } );
    
    [[ maybe_unused ]] auto const success{ evaluator.expression( expr ) };
    data_object< std::string > obj{ "value1" };  // Match on first value

    for ( auto _ : state )
    {
        [[ maybe_unused ]] auto const result{ evaluator.evaluate( obj ) };
        benchmark::DoNotOptimize( result );
        benchmark::ClobberMemory();
    }
}

BENCHMARK( ShortCircuitTest_FirstMatch_10Values );

void ShortCircuitTest_LastMatch_10Values( benchmark::State& state )
{
    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field", &data_object< std::string >::value )
        }
    };

    std::string expr = generate_in_expression( "field", 
        { "value1", "value2", "value3", "value4", "value5",
          "value6", "value7", "value8", "value9", "value10" } );
    
    [[ maybe_unused ]] auto const success{ evaluator.expression( expr ) };
    data_object< std::string > obj{ "value10" };  // Match on last value

    for ( auto _ : state )
    {
        [[ maybe_unused ]] auto const result{ evaluator.evaluate( obj ) };
        benchmark::DoNotOptimize( result );
        benchmark::ClobberMemory();
    }
}

BENCHMARK( ShortCircuitTest_LastMatch_10Values );

void ShortCircuitTest_NoMatch_10Values( benchmark::State& state )
{
    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field", &data_object< std::string >::value )
        }
    };

    std::string expr = generate_in_expression( "field", 
        { "value1", "value2", "value3", "value4", "value5",
          "value6", "value7", "value8", "value9", "value10" } );
    
    [[ maybe_unused ]] auto const success{ evaluator.expression( expr ) };
    data_object< std::string > obj{ "nomatch" };  // No match

    for ( auto _ : state )
    {
        [[ maybe_unused ]] auto const result{ evaluator.evaluate( obj ) };
        benchmark::DoNotOptimize( result );
        benchmark::ClobberMemory();
    }
}

BENCHMARK( ShortCircuitTest_NoMatch_10Values );

BENCHMARK_MAIN();