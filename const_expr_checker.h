#pragma once
#include "node.h"
#include "cl.h"

using ConstOpt = std::optional<Value>;

ConstOpt checkConstantExpressions(Node *p);