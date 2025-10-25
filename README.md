# PLP Framework - Phenomenological Lensing Principle
## By Nnamdi Michael Okpala
### Phenodata Structure Documentation: AVL-Trie Hybrid for Government ID Systems

---

## Overview: Phenodata Structure
The **Phenodata Structure** within the PLP framework represents a hybrid data architecture designed to encode both the *objective state* of information and its *subjective frame of reference*. It merges the balancing capabilities of an **AVL tree** with the associative indexing of a **Trie**, enabling efficient lookup while preserving phenomenological context — the “how” and “why” behind the data’s existence.

### Core Concept
Each **phenodata node** acts as a minimal cognitive cell — a record of value, type, and context — allowing the system to track not just data, but its meaning relative to its observer or issuer.

A node can store:
- **Primitive types**: `char`, `int`, `bool`
- **Strong/Weak typed values**: flexible dynamic typing
- **Phenomenological context**: subjective metadata linking to an observer or event

---

## Architecture Components

### 1. AVL-Trie Hybrid Structure
A PhenodataNode blends the balancing logic of an AVL tree with the prefix-matching power of a Trie:

```rust
pub struct PhenodataNode<T: Ord + Copy> {
    // Core data
    pub value: T,
    pub node_type: DataType,

    // AVL balancing
    pub height: i32,
    pub left: Option<Box<PhenodataNode<T>>>,
    pub right: Option<Box<PhenodataNode<T>>>,

    // Trie indexing
    pub children: HashMap<char, Box<PhenodataNode<T>>>,
    pub is_terminal: bool,

    // Phenomenological reference
    pub phenomenohog: Option<PhenomenohogBlock>,
}
```

This architecture ensures *temporal and semantic stability* while allowing multi-perspective traversal.

---

### 2. Frame of Reference for Government IDs
The frame defines the context of issuance, validation, and verification:

```rust
pub struct GovernmentIDFrame {
    pub id_type: IDType,
    pub issuing_authority: String,
    pub validation_status: bool,
    pub phenomenohog_context: PhenomenohogBlock,
}

pub enum IDType {
    NationalInsurance(String),
    SocialSecurity(String),
    BirthCertificate { number: String, district: String, year: u32 },
    PassportNumber(String),
    DriverLicense(String),
}
```

Each ID is a context-bearing artifact — its verification event and issuer become part of its identity.

---

### 3. Phenomenohog Subject Context
The **PhenomenohogBlock** provides a record of subjective interactions — who verified what, under which frame, and when.

```rust
pub struct PhenomenohogBlock {
    pub session: String,
    pub scope: String, // person | instance | context
    pub type_field: String,
    pub frame_of_reference: String,
    pub timestamp: DateTime<Utc>,
    pub diram_state: Diram, // Null | Partial | Collapse | Intact
}
```

This allows identity data to retain the *memory of its observation* — a phenomenological audit trail.

---

## Coherence and the PLP Principle
PLP (Phenomenological Lensing Principle) views coherence as the measure of how consistently a system maintains internal agreement between its modeled behavior and its lived data. Each node’s **coherence operator** verifies that its context aligns with its neighbors and parent frames.

- **PLP Coherence:** Self-consistency of data and context
- **Coherence Operator:** Mechanism that maintains agreement across frames
- **OEEOs (Observation & Error Evaluation Operators):** Agents that detect and mitigate decoherence before it cascades

Together, they allow the system to remain logically intact while evolving.

---

## Token Type System
A token layer supports encoding, validation, and pruning decisions:

```rust
pub enum TokenType {
    CharToken(char),
    IntToken(i32),
    BoolToken(bool),
    NIToken { prefix: [char; 2], numbers: [u8; 6], suffix: char },
    SSNToken { area: u16, group: u8, serial: u16 },
}

pub struct TokenValue {
    pub token_type: TokenType,
    pub raw_value: Vec<u8>,
    pub encoded_value: Vec<u8>,
    pub memory_weight: f32,
}
```

---

## Phenological Memory and Rotation
AVL rotations preserve structure while extending context inheritance:

```rust
impl<T: Ord + Copy> PhenodataNode<T> {
    fn rotate_right_with_context(mut y: Box<PhenodataNode<T>>) -> Box<PhenodataNode<T>> {
        let mut x = y.left.take().unwrap();
        if let Some(y_ctx) = &y.phenomenohog {
            if let Some(x_ctx) = &mut x.phenomenohog {
                x_ctx.frame_of_reference.push_str(&format!(",rotation:LL,parent:{}", y_ctx.session));
            }
        }
        y.left = x.right.take();
        y.update_height();
        x.right = Some(y);
        x.update_height();
        x
    }
}
```

This maintains coherence across rotations, treating rebalancing as a memory-preserving act.

---

## Cross-Language Integration

### Go (Gosilang)
```go
type PhenodataNode struct {
    Value        interface{}
    NodeType     string
    Height       int32
    Left, Right  *PhenodataNode
    Children     map[rune]*PhenodataNode
    IsTerminal   bool
    Phenomenohog *PhenomenohogBlock
}
```

### Riftbridge Adapter
```rust
pub struct RiftbridgeAdapter {
    pub phenodata_root: Box<PhenodataNode<char>>,
    pub span_registry: HashMap<String, SpanMarker>,
    pub germ_data_cache: Vec<u8>,
}
```

Riftbridge translates phenodata into transportable semantic payloads for distributed verification.

---

## Example: National Insurance Number Validation
```rust
let ni_frame = GovernmentIDFrame {
    id_type: IDType::NationalInsurance("AB123456C".to_string()),
    issuing_authority: "HMRC_UK".to_string(),
    validation_status: true,
    phenomenohog_context: PhenomenohogBlock {
        session: "validation_session_001".to_string(),
        scope: "person".to_string(),
        type_field: "government_id".to_string(),
        frame_of_reference: "subject:john_doe,verifier:hmrc_system,context:employment_verification".to_string(),
        timestamp: Utc::now(),
        diram_state: Diram::Intact,
    },
};

phenodata_tree.insert_with_frame("AB123456C", ni_frame);
```

---

## Summary
The PLP Phenodata architecture:
1. Encapsulates atomic context-aware data units
2. Uses AVL balancing for performance
3. Leverages Trie indexing for text search
4. Preserves phenomenological meaning
5. Provides coherence monitoring and correction
6. Integrates cryptographic verification and OEEO-driven stability

It’s a data structure that remembers *who saw what, when, and why* — a foundation for coherent, observer-inclusive computation.
