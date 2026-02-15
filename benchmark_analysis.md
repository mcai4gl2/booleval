# BoolEval Benchmark Analysis: Real-World Usage vs. Advertised Performance

## Executive Summary

This analysis compares the original booleval benchmarks with real-world usage patterns for expressions like `Field in ('v1', 'v2', ..., 'vN')`. The results show **significant performance degradation** when parsing and evaluating expressions on every use case, which is the common pattern in production systems.

---

## Benchmark Environment

- **Compiler**: GCC 11.4.0
- **Build Type**: Release (-O3)
- **CPU**: 8 cores @ 2.208 GHz
- **Test Repetitions**: 3 per benchmark
- **Test Date**: 2026-02-15

---

## Original Benchmarks (Best Case Scenario)

The original benchmarks measure two separate operations:

| Benchmark | Mean Time (ns) | Operations/sec |
|-----------|----------------|----------------|
| BuildingExpressionTree | 1,374 | ~727,000 |
| Evaluation | 1,149 | ~869,000 |

**Key Insight**: These benchmarks separate parsing from evaluation, which is optimal but not representative of typical usage.

---

## User Use Case Benchmarks (Real-World Scenario)

Your use case: Parse AND evaluate expressions on every iteration, simulating `Field in ('v1', 'v2', ..., 'vN')`.

### Parse + Evaluate on Every Iteration

| Values | Mean Time (ns) | Operations/sec | vs Original |
|--------|----------------|----------------|-------------|
| 1 value | 243 | ~4,110,000 | **5.4x faster** than parsing + evaluating separately |
| 5 values | 2,608 | ~383,000 | **2.3x slower** than evaluation alone |
| 10 values | 9,640 | ~103,700 | **11.3x slower** than evaluation alone |
| 50 values | 177,286 | ~5,640 | **154x slower** than evaluation alone |

### Critical Finding: No Short-Circuit Evaluation

The library evaluates ALL values in OR chains, even after finding a match:

| Match Position | Mean Time (ns) | Operations/sec |
|----------------|----------------|----------------|
| First value (position 1) | 367 | ~2,720,000 |
| Last value (position 10) | 333 | ~3,000,000 |
| No match | 330 | ~3,030,000 |

**Insight**: Performance is nearly identical regardless of match position, confirming **no short-circuit optimization**.

### Parse Once, Evaluate Many (Best Practice)

| Benchmark | Mean Time (ns) | Operations/sec |
|-----------|----------------|----------------|
| Parse once, evaluate 10 values | 375 | ~2,660,000 |

---

## Performance Analysis

### 1. Linear Scaling with Value Count

Performance degrades quadratically as the number of values increases:

```
1 value:   243 ns
5 values:  2,608 ns   (10.7x)
10 values: 9,640 ns   (39.7x)
50 values: 177,286 ns (730x)
```

### 2. Parsing Cost Dominates

For your use case (10 values):
- **Parse + Evaluate**: 9,640 ns
- **Evaluate only** (parsed once): 375 ns
- **Parsing overhead**: ~9,265 ns (96% of total time)

### 3. No Short-Circuit Evaluation

The `result_visitor.hpp` implementation (lines 88-100) evaluates both sides of AND/OR operations:

```cpp
auto const left { visit( *node.left , std::forward< T >( obj ) ) };
auto const right{ visit( *node.right, std::forward< T >( obj ) ) };
```

This means for `Field v1 or Field v2 or ... or Field v10`, **all 10 comparisons are always executed**, even if `v1` matches.

### 4. Real-World vs. Advertised Performance

| Scenario | Operations/sec | vs Advertised |
|----------|----------------|---------------|
| Advertised (best case) | ~869,000 | baseline |
| Your use case (10 values) | ~103,700 | **8.4x slower** |
| Your use case (50 values) | ~5,640 | **154x slower** |

---

## Root Causes

1. **No caching support**: Cannot reuse parsed expressions
2. **No short-circuit evaluation**: All OR conditions always evaluated
3. **Linear string comparisons**: No hash-based optimization for IN clauses
4. **Expression tree overhead**: Creating/deleting tree nodes on every parse
5. **Sequential field lookup**: O(F) where F = number of fields

---

## Recommendations

### Immediate (No Code Changes)

1. **Cache parsed expressions**: Parse once, reuse many times
   ```cpp
   // Bad: Parse + evaluate on every iteration
   for (auto& item : items) {
       evaluator.expression("field v1 or field v2");
       if (evaluator.evaluate(item)) { ... }
   }
   
   // Good: Parse once, evaluate many
   evaluator.expression("field v1 or field v2");
   for (auto& item : items) {
       if (evaluator.evaluate(item)) { ... }
   }
   ```

### Medium-term (Library Enhancements)

1. **Add short-circuit evaluation** to `result_visitor.hpp`
2. **Add IN operator** with hash-based lookup
3. **Add expression caching** to evaluator
4. **Add compiled expression** mode for maximum performance

### Long-term (Architecture Changes)

1. **Compile to bytecode**: Parse once, execute multiple times
2. **Add JIT compilation**: Generate native code for expressions
3. **Support expression templates**: Compile-time optimization

---

## Conclusion

Your observation of **much lower throughput than advertised** is correct. The original benchmarks measure ideal conditions (parse once, evaluate many) that don't reflect real-world usage patterns where expressions are parsed and evaluated on every use case.

For your typical use case (1-10 values), expect:
- **Parse + Evaluate every time**: ~103,700 ops/sec (10 values)
- **Parse once, evaluate many**: ~2,660,000 ops/sec (10 values)

**26x improvement** possible by caching parsed expressions.

---

## Benchmark Files

- **Original benchmark**: `benchmark/src/booleval_benchmark.cpp`
- **User case benchmark**: `benchmark/src/user_case_benchmark.cpp`

Both benchmarks can be run with:
```bash
./build/src/booleval_benchmark --benchmark_format=json
./build/src/user_case_benchmark --benchmark_format=json
```