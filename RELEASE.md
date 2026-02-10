# Release Notes

## v0.2.0

### Performance

Benchmark: `time bin/es typescript-test.js` (TypeScript compiler self-host, ~1.2MB source)

- **v0.1.x**: ~12.5s
- **v0.2.0**: ~9.1s
- **Improvement**: ~27%

### Optimizations

#### Handle System

- **HandleScope::Add fast path**: Added static slots for fixed stack-type singletons (Undefined, Null, Bool), avoiding `BlockStack` overhead for the most common values.
- **HandleScope constant dedup**: Replaced linear scan with open-addressing hash table (`ConstantHandleMap`) for CONST handle deduplication.
- **BlockStack::Rewind fast path**: Same-block rewind now directly resets the cursor pointer without touching the vector.

#### Property Access

- **PropertyMap::GetDataValue**: New method that returns raw `JSValue*` directly from hash probe, skipping `StackPropertyDescriptor` construction for data-only lookups.
- **Get\_\_Base raw pointer prototype chain walk**: Rewrote the core property lookup (`[[Get]]`) to traverse the prototype chain using raw `JSObject*` pointers instead of `Handle<JSObject>`, eliminating Handle creation per prototype level.
- **GetProperty raw pointer walk**: Similarly converted `[[GetProperty]]` to use raw pointers for prototype chain traversal.
- **PropertyMap::hashmap\_raw()**: Added raw accessor returning `HashMapV2*` directly, bypassing Handle creation in hot lookup paths.
- **JSObject::Prototype\_raw()**: Added raw accessor returning `JSValue*` for prototype, used in raw pointer chain walks.

#### Hash Map

- **HashMapV2::Probe pointer-first comparison**: Probe now checks pointer equality (`e.key == key`) before falling back to hash + content comparison, exploiting the common case where the same `String*` object is reused.
- **HashEqualStringEqual simplification**: Removed redundant size check when hashes already match (hash encodes size for `JS_STRING` type).

#### Scope Chain

- **EnvironmentRecord::outer\_raw()**: Added raw accessor for scope chain traversal without Handle creation.
- **GetIdentifierReferenceAndGetValue / PutValue**: Rewrote scope chain walk using `EnvironmentRecord*` raw pointer loop. For declarative environments, directly probes bindings and inlines `GetBindingValue` / `SetMutableBinding`, avoiding Handle creation entirely on the common path.
- **GetIdentifierReference**: Converted from recursive to iterative with raw pointers.

#### GC / ExtracGC

- **FunctionDeclarativeEnvironmentRecord bug fix**: Fixed uninitialized `id` member (constructor parameter was shadowing the field). Also fixed `operator[]` to use the correct static `env_recs` array with `kMaxNumPushed * id` offset.
- **ExtracGC cached lookup**: Cache `FunctionDeclarativeEnvironmentRecord*` pointer on AST node (`ProgramOrFunctionBody::cached_env_rec_`), eliminating `std::unordered_map::find()` on every function call/return. Profile confirmed `__hash_table::find` dropped from 1.1% to 0%.

#### Expression Evaluation

- **EvalExpressionAndGetValue switch dispatch**: Directly dispatches common expression types (identifiers, literals, binary, unary, ternary) to their evaluators without going through `EvalExpression` + `GetValue`, saving one level of indirection.
- **GetValue fast path**: Reordered checks to test `IsObject` first with `likely()` hint, as object property references dominate.
- **Completion factory methods**: `Completion::Normal()`, `Completion::Throw()`, `Completion::Return()` avoid redundant field initialization.

#### Runtime

- **Context stack reserve**: Pre-allocate context stack capacity to reduce reallocation.
- **Error::InitOk / AddContext**: Streamlined error initialization and context setup paths.
