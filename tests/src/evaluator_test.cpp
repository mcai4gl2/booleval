/*
 * Copyright (c) 2019, Marin Peko
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <gtest/gtest.h>
#include <booleval/evaluator.hpp>

namespace
{

    template< typename T >
    class foo
    {
    public:
        foo()             : value_{}        {}
        foo( T && value ) : value_{ value } {}

        void value( T && value ) { value_ = value; }

        T value() const noexcept { return value_; }

    private:
        T value_{};
    };

    template< typename T, typename U >
    class bar
    {
    public:
        bar( T && value_1, U && value_2 )
        : value_1_{ value_1 }
        , value_2_{ value_2 }
        {}

        void value_1( T && value ) { value_1_ = value; }
        void value_2( U && value ) { value_2_ = value; }

        T value_1() const noexcept { return value_1_; }
        U value_2() const noexcept { return value_2_; }

    private:
        T value_1_{};
        U value_2_{};
    };

} // namespace

TEST( EvaluatorTest, DefaultConstructor )
{
    booleval::evaluator evaluator;

    ASSERT_FALSE( evaluator.is_activated() );
}

TEST( EvaluatorTest, EmptyExpression )
{
    booleval::evaluator evaluator;

    ASSERT_TRUE ( evaluator.expression("")                            );
    ASSERT_FALSE( evaluator.is_activated()                            );
    ASSERT_FALSE( evaluator.evaluate( foo< std::uint8_t >{} ).success );
}

TEST( EvaluatorTest, MissingParenthesesExpression )
{
    booleval::evaluator evaluator;

    ASSERT_FALSE( evaluator.expression( "(field_x foo or field_y bar" ) );
    ASSERT_FALSE( evaluator.is_activated()                              );
    ASSERT_FALSE( evaluator.evaluate( foo< std::uint8_t >{} ).success   );
}

TEST( EvaluatorTest, MultipleFieldsInRowExpression )
{
    booleval::evaluator evaluator;

    ASSERT_FALSE( evaluator.expression( "field_x foo field_y" )       );
    ASSERT_FALSE( evaluator.is_activated()                            );
    ASSERT_FALSE( evaluator.evaluate( foo< std::uint8_t >{} ).success );
}

TEST( EvaluatorTest, EqualToOperator )
{
    foo< std::string > x{ "foo" };
    foo< std::string > y{ "bar" };

    booleval::evaluator evaluator
    {
        booleval::make_field( "field", &foo< std::string >::value )
    };

    {
        ASSERT_TRUE ( evaluator.expression( "field foo" ) );
        ASSERT_TRUE ( evaluator.is_activated()            );
        ASSERT_TRUE ( evaluator.evaluate( x ).success     );
        ASSERT_FALSE( evaluator.evaluate( y ).success     );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field eq foo" ) );
        ASSERT_TRUE ( evaluator.is_activated()               );
        ASSERT_TRUE ( evaluator.evaluate( x ).success        );
        ASSERT_FALSE( evaluator.evaluate( y ).success        );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field == foo" ) );
        ASSERT_TRUE ( evaluator.is_activated()               );
        ASSERT_TRUE ( evaluator.evaluate( x ).success        );
        ASSERT_FALSE( evaluator.evaluate( y ).success        );
    }
    {
        x.value( "foo foo" );

        ASSERT_TRUE ( evaluator.expression( "field == \"foo foo\"" ) );
        ASSERT_TRUE ( evaluator.is_activated()                       );
        ASSERT_TRUE ( evaluator.evaluate( x ).success                );
        ASSERT_FALSE( evaluator.evaluate( y ).success                );
    }
}

TEST( EvaluatorTest, NotEqualToOperator )
{
    foo< std::string > x{ "foo" };
    foo< std::string > y{ "bar" };

    booleval::evaluator evaluator
    {
        booleval::make_field( "field", &foo< std::string >::value )
    };

    {
        ASSERT_TRUE ( evaluator.expression( "field neq foo" ) );
        ASSERT_TRUE ( evaluator.is_activated()                );
        ASSERT_FALSE( evaluator.evaluate( x ).success         );
        ASSERT_TRUE ( evaluator.evaluate( y ).success         );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field != foo" ) );
        ASSERT_TRUE ( evaluator.is_activated()               );
        ASSERT_FALSE( evaluator.evaluate( x ).success        );
        ASSERT_TRUE ( evaluator.evaluate( y ).success        );
    }
    {
        x.value( "foo foo" );

        ASSERT_TRUE ( evaluator.expression( "field != \"foo foo\"" ) );
        ASSERT_TRUE ( evaluator.is_activated()                       );
        ASSERT_FALSE( evaluator.evaluate( x ).success                );
        ASSERT_TRUE ( evaluator.evaluate( y ).success                );
    }
}

TEST( EvaluatorTest, GreaterThanOperator )
{
    foo< float > x{ 1.22f };
    foo< float > y{ 1.24f };

    foo< std::string > m{ "1000" };
    foo< std::string > n{ "50"   };

    booleval::evaluator evaluator_digits
    {
        booleval::make_field( "field", &foo< float >::value )
    };

    booleval::evaluator evaluator_strings
    {
        booleval::make_field( "field", &foo< std::string >::value )
    };

    {
        ASSERT_TRUE ( evaluator_digits.expression( "field gt 1.23" ) );
        ASSERT_TRUE ( evaluator_digits.is_activated()                );
        ASSERT_FALSE( evaluator_digits.evaluate( x ).success         );
        ASSERT_TRUE ( evaluator_digits.evaluate( y ).success         );
    }
    {
        ASSERT_TRUE ( evaluator_digits.expression( "field > 1.23" ) );
        ASSERT_TRUE ( evaluator_digits.is_activated()               );
        ASSERT_FALSE( evaluator_digits.evaluate( x ).success        );
        ASSERT_TRUE ( evaluator_digits.evaluate( y ).success        );
    }
    {
        ASSERT_TRUE ( evaluator_strings.expression( "field gt \"200\"" ) );
        ASSERT_TRUE ( evaluator_strings.is_activated()                   );
        ASSERT_FALSE( evaluator_strings.evaluate( m ).success            );
        ASSERT_TRUE ( evaluator_strings.evaluate( n ).success            );
    }
    {
        ASSERT_TRUE ( evaluator_strings.expression( "field > \"200\"" ) );
        ASSERT_TRUE ( evaluator_strings.is_activated()                  );
        ASSERT_FALSE( evaluator_strings.evaluate( m ).success           );
        ASSERT_TRUE ( evaluator_strings.evaluate( n ).success           );
    }
}

TEST( EvaluatorTest, GreaterThanOrEqualToOperator )
{
    foo< double > x{ 1.234567 };
    foo< double > y{ 2.345678 };
    foo< double > z{ 0.123456 };

    booleval::evaluator evaluator
    {
        booleval::make_field( "field", &foo< double >::value )
    };

    {
        ASSERT_TRUE ( evaluator.expression( "field geq 1.234567" ) );
        ASSERT_TRUE ( evaluator.is_activated()                     );
        ASSERT_TRUE ( evaluator.evaluate( x ).success              );
        ASSERT_TRUE ( evaluator.evaluate( y ).success              );
        ASSERT_FALSE( evaluator.evaluate( z ).success              );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field >= 1.234567" ) );
        ASSERT_TRUE ( evaluator.is_activated()                    );
        ASSERT_TRUE ( evaluator.evaluate( x ).success             );
        ASSERT_TRUE ( evaluator.evaluate( y ).success             );
        ASSERT_FALSE( evaluator.evaluate( z ).success             );
    }
}

TEST( EvaluatorTest, LessThanOperator )
{
    foo< unsigned > x{ 1 };
    foo< unsigned > y{ 3 };

    foo< std::string > m{ "1000" };
    foo< std::string > n{ "50"   };

    booleval::evaluator evaluator_digits
    {
        booleval::make_field( "field", &foo< unsigned >::value )
    };

    booleval::evaluator evaluator_strings
    {
        booleval::make_field( "field", &foo< std::string >::value )
    };

    {
        ASSERT_TRUE ( evaluator_digits.expression( "field lt 2" ) );
        ASSERT_TRUE ( evaluator_digits.is_activated()             );
        ASSERT_TRUE ( evaluator_digits.evaluate( x ).success      );
        ASSERT_FALSE( evaluator_digits.evaluate( y ).success      );
    }
    {
        ASSERT_TRUE ( evaluator_digits.expression( "field < 2" ) );
        ASSERT_TRUE ( evaluator_digits.is_activated()            );
        ASSERT_TRUE ( evaluator_digits.evaluate( x ).success     );
        ASSERT_FALSE( evaluator_digits.evaluate( y ).success     );
    }
    {
        ASSERT_TRUE ( evaluator_strings.expression( "field lt \"200\"" ) );
        ASSERT_TRUE ( evaluator_strings.is_activated()                   );
        ASSERT_TRUE ( evaluator_strings.evaluate( m ).success            );
        ASSERT_FALSE( evaluator_strings.evaluate( n ).success            );
    }
    {
        ASSERT_TRUE ( evaluator_strings.expression( "field < \"200\"" ) );
        ASSERT_TRUE ( evaluator_strings.is_activated()                  );
        ASSERT_TRUE ( evaluator_strings.evaluate( m ).success           );
        ASSERT_FALSE( evaluator_strings.evaluate( n ).success           );
    }
}

TEST( EvaluatorTest, LessThanOrEqualToOperator )
{
    foo< double > x{ 1.234567 };
    foo< double > y{ 2.345678 };
    foo< double > z{ 0.123456 };

    booleval::evaluator evaluator
    {
        booleval::make_field( "field", &foo< double >::value )
    };

    {
        ASSERT_TRUE ( evaluator.expression( "field leq 1.234567" ) );
        ASSERT_TRUE ( evaluator.is_activated()                     );
        ASSERT_TRUE ( evaluator.evaluate( x ).success              );
        ASSERT_FALSE( evaluator.evaluate( y ).success              );
        ASSERT_TRUE ( evaluator.evaluate( z ).success              );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field <= 1.234567" ) );
        ASSERT_TRUE ( evaluator.is_activated()                    );
        ASSERT_TRUE ( evaluator.evaluate( x ).success             );
        ASSERT_FALSE( evaluator.evaluate( y ).success             );
        ASSERT_TRUE ( evaluator.evaluate( z ).success             );
    }
}

TEST( EvaluatorTest, AndOperator )
{
    bar< unsigned, std::string > x{ 1, "bar"     };
    bar< unsigned, std::string > y{ 3, "bar bar" };

    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field_1", &bar< unsigned, std::string >::value_1 ),
            booleval::make_field( "field_2", &bar< unsigned, std::string >::value_2 )
        }
    };

    {
        ASSERT_TRUE ( evaluator.expression( "field_1 1 and field_2 bar" ) );
        ASSERT_TRUE ( evaluator.is_activated()                            );
        ASSERT_TRUE ( evaluator.evaluate( x ).success                     );
        ASSERT_FALSE( evaluator.evaluate( y ).success                     );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field_1 eq 1 and field_2 eq bar" ) );
        ASSERT_TRUE ( evaluator.is_activated()                                  );
        ASSERT_TRUE ( evaluator.evaluate( x ).success                           );
        ASSERT_FALSE( evaluator.evaluate( y ).success                           );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field_1 == 1 and field_2 == bar" ) );
        ASSERT_TRUE ( evaluator.is_activated()                                  );
        ASSERT_TRUE ( evaluator.evaluate( x ).success                           );
        ASSERT_FALSE( evaluator.evaluate( y ).success                           );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field_1 == 1 && field_2 == bar" ) );
        ASSERT_TRUE ( evaluator.is_activated()                                 );
        ASSERT_TRUE ( evaluator.evaluate( x ).success                          );
        ASSERT_FALSE( evaluator.evaluate( y ).success                          );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field_1 == 3 && field_2 == bar" ) );
        ASSERT_TRUE ( evaluator.is_activated()                                 );
        ASSERT_FALSE( evaluator.evaluate( x ).success                          );
        ASSERT_FALSE( evaluator.evaluate( y ).success                          );
    }
}

TEST( EvaluatorTest, OrOperator )
{
    bar< unsigned, std::string > x{ 1, "bar"     };
    bar< unsigned, std::string > y{ 3, "bar bar" };

    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field_1", &bar< unsigned, std::string >::value_1 ),
            booleval::make_field( "field_2", &bar< unsigned, std::string >::value_2 )
        }
    };

    {
        ASSERT_TRUE ( evaluator.expression( "field_1 1 or field_1 2" ) );
        ASSERT_TRUE ( evaluator.is_activated()                         );
        ASSERT_TRUE ( evaluator.evaluate( x ).success                  );
        ASSERT_FALSE( evaluator.evaluate( y ).success                  );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field_1 eq 1 or field_1 eq 2" ) );
        ASSERT_TRUE ( evaluator.is_activated()                               );
        ASSERT_TRUE ( evaluator.evaluate( x ).success                        );
        ASSERT_FALSE( evaluator.evaluate( y ).success                        );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field_1 == 1 or field_1 == 2" ) );
        ASSERT_TRUE ( evaluator.is_activated()                               );
        ASSERT_TRUE ( evaluator.evaluate( x ).success                        );
        ASSERT_FALSE( evaluator.evaluate( y ).success                        );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field_1 1 || field_1 2" ) );
        ASSERT_TRUE ( evaluator.is_activated()                         );
        ASSERT_TRUE ( evaluator.evaluate( x ).success                  );
        ASSERT_FALSE( evaluator.evaluate( y ).success                  );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field_1 eq 1 || field_1 eq 2" ) );
        ASSERT_TRUE ( evaluator.is_activated()                               );
        ASSERT_TRUE ( evaluator.evaluate( x ).success                        );
        ASSERT_FALSE( evaluator.evaluate( y ).success                        );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field_1 == 1 || field_1 == 2" ) );
        ASSERT_TRUE ( evaluator.is_activated()                               );
        ASSERT_TRUE ( evaluator.evaluate( x ).success                        );
        ASSERT_FALSE( evaluator.evaluate( y ).success                        );
    }
    {
        ASSERT_TRUE( evaluator.expression( "field_1 == 1 || field_2 == \"bar bar\"" ) );
        ASSERT_TRUE( evaluator.is_activated()                                         );
        ASSERT_TRUE( evaluator.evaluate( x ).success                                  );
        ASSERT_TRUE( evaluator.evaluate( y ).success                                  );
    }
    {
        ASSERT_TRUE( evaluator.expression( "field_1 == 3 || field_2 == bar" ) );
        ASSERT_TRUE( evaluator.is_activated()                                 );
        ASSERT_TRUE( evaluator.evaluate( x ).success                          );
        ASSERT_TRUE( evaluator.evaluate( y ).success                          );
    }
}

TEST( EvaluatorTest, MultipleOperators )
{
    bar< std::string, unsigned > x{ "foo", 1 };
    bar< std::string, unsigned > y{ "bar", 2 };
    bar< std::string, unsigned > m{ "baz", 1 };
    bar< std::string, unsigned > n{ "qux", 2 };

    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field_1", &bar< std::string, unsigned >::value_1 ),
            booleval::make_field( "field_2", &bar< std::string, unsigned >::value_2 )
        }
    };

    {
        ASSERT_TRUE ( evaluator.expression( "(field_1 foo and field_2 1)" ) );
        ASSERT_TRUE ( evaluator.is_activated()        );
        ASSERT_TRUE ( evaluator.evaluate( x ).success );
        ASSERT_FALSE( evaluator.evaluate( y ).success );
        ASSERT_FALSE( evaluator.evaluate( m ).success );
        ASSERT_FALSE( evaluator.evaluate( n ).success );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "field_1 foo and field_2 1 and field_1 bar" ) );
        ASSERT_TRUE ( evaluator.is_activated()        );
        ASSERT_FALSE( evaluator.evaluate( x ).success );
        ASSERT_FALSE( evaluator.evaluate( y ).success );
        ASSERT_FALSE( evaluator.evaluate( m ).success );
        ASSERT_FALSE( evaluator.evaluate( n ).success );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "(field_1 foo or field_1 bar) and (field_2 2 or field_2 1)" ) );
        ASSERT_TRUE ( evaluator.is_activated()        );
        ASSERT_TRUE ( evaluator.evaluate( x ).success );
        ASSERT_TRUE ( evaluator.evaluate( y ).success );
        ASSERT_FALSE( evaluator.evaluate( m ).success );
        ASSERT_FALSE( evaluator.evaluate( n ).success );
    }
    {
        ASSERT_TRUE ( evaluator.expression( "(field_1 foo and field_2 1) or (field_1 qux and field_2 2)" ) );
        ASSERT_TRUE ( evaluator.is_activated()        );
        ASSERT_TRUE ( evaluator.evaluate( x ).success );
        ASSERT_FALSE( evaluator.evaluate( y ).success );
        ASSERT_FALSE( evaluator.evaluate( m ).success );
        ASSERT_TRUE ( evaluator.evaluate( n ).success );
    }
}

TEST( EvaluatorTest, DifferentClasses )
{
    foo< unsigned              > x{ 1        };
    bar< unsigned, std::string > y{ 2, "bar" };

    booleval::evaluator evaluator
    {
        {
            booleval::make_field( "field_1", &foo< unsigned              >::value   ),
            booleval::make_field( "field_2", &bar< unsigned, std::string >::value_2 )
        }
    };

    {
        ASSERT_TRUE ( evaluator.expression( "field_1 one and field_2 2" ) );
        ASSERT_TRUE ( evaluator.is_activated()                            );
        ASSERT_FALSE( evaluator.evaluate( x ).success                     );
        ASSERT_FALSE( evaluator.evaluate( y ).success                     );
    }
}

TEST( EvaluatorTest, UnknownField )
{
    foo< unsigned > x{ 1 };

    booleval::evaluator evaluator
    {
        booleval::make_field( "field", &foo< unsigned >::value )
    };

    {
        ASSERT_TRUE ( evaluator.expression( "unknown_field 1" )        );
        ASSERT_TRUE ( evaluator.is_activated()                         );

        auto const result{ evaluator.evaluate( x ) };
        ASSERT_FALSE( result.success                  );
        ASSERT_EQ   ( result.message, "Unknown field" );
    }
}

TEST( EvaluatorTest, EmptyStringInMiddleOfExpression )
{
    // Test that empty string in the middle of an expression doesn't cause infinite loop
    booleval::evaluator evaluator;

    // Test case 1: Empty string with AND operator
    // This should handle gracefully without infinite loop
    [[maybe_unused]] auto result1 = evaluator.expression( "field eq '' and other_field bar" );
    
    // Test case 2: Empty string with OR operator
    booleval::evaluator evaluator2;
    [[maybe_unused]] auto result2 = evaluator2.expression( "field eq '' or other_field bar" );

    // Test case 3: Multiple empty strings
    booleval::evaluator evaluator3;
    [[maybe_unused]] auto result3 = evaluator3.expression( "field eq '' and field2 eq ''" );

    // Test case 4: Empty string with parentheses
    booleval::evaluator evaluator4;
    [[maybe_unused]] auto result4 = evaluator4.expression( "(field eq '') and (other_field bar)" );

    // All these should complete without infinite loop
    // We don't care about the result validity, just that it completes
    SUCCEED();
}

TEST( EvaluatorTest, DataObjectEvaluationExample )
{
    // Example: How to use booleval with a custom data object
    
    // Step 1: Define your data object with getter methods
    class Person
    {
    public:
        Person(std::string name, unsigned age)
            : name_(std::move(name)), age_(age) {}
        
        std::string const& name() const noexcept { return name_; }
        unsigned age() const noexcept { return age_; }
        
    private:
        std::string name_;
        unsigned age_{0};
    };
    
    // Step 2: Create an evaluator and register fields using make_field()
    booleval::evaluator evaluator
    {
        {
            booleval::make_field("name", &Person::name),
            booleval::make_field("age", &Person::age)
        }
    };
    
    // Step 3: Set an expression to evaluate
    // This expression matches persons named "John" who are 25 years old
    ASSERT_TRUE(evaluator.expression("name John and age 25"));
    ASSERT_TRUE(evaluator.is_activated());
    
    // Step 4: Evaluate objects against the expression
    Person person1{"John", 25};
    Person person2{"Jane", 25};
    Person person3{"John", 30};
    Person person4{"John", 25};
    
    // Only person1 and person4 match both conditions
    auto result1 = evaluator.evaluate(person1);
    ASSERT_TRUE(result1.success);
    
    auto result2 = evaluator.evaluate(person2);
    ASSERT_FALSE(result2.success); // name doesn't match
    
    auto result3 = evaluator.evaluate(person3);
    ASSERT_FALSE(result3.success); // age doesn't match
    
    auto result4 = evaluator.evaluate(person4);
    ASSERT_TRUE(result4.success);
    
    // Example with different operators
    // Match persons older than 20 or named "Jane"
    [[maybe_unused]] auto expr_result2 = evaluator.expression("age gt 20 or name Jane");
    
    auto result5 = evaluator.evaluate(person1); // age 25 > 20, matches
    ASSERT_TRUE(result5.success);
    
    auto result6 = evaluator.evaluate(person2); // name "Jane", matches
    ASSERT_TRUE(result6.success);
    
    auto result7 = evaluator.evaluate(person3); // age 30 > 20, matches
    ASSERT_TRUE(result7.success);
    
    // Example with parentheses for grouping
    // Match persons named "John" who are under 30
    [[maybe_unused]] auto expr_result3 = evaluator.expression("(name John and age lt 30)");
    
    auto result8 = evaluator.evaluate(person1); // John, 25 - matches
    ASSERT_TRUE(result8.success);
    
    auto result9 = evaluator.evaluate(person3); // John, 30 - age not lt 30
    ASSERT_FALSE(result9.success);
    
    auto result10 = evaluator.evaluate(person4); // John, 25 - matches
    ASSERT_TRUE(result10.success);
}
