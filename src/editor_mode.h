// Copyright 2025 Ruben Berenguel

#pragma once

// This is not part of the editor to avoid a circular dependency,
// since Output also uses it.

enum class EditorMode { kNormal, kInsert, kCommandLineMode };
