#!/bin/bash
# PreToolUse hook (Bash): blocks `git commit` until the embedded-review skill has approved
# exactly the changes that are currently staged. The skill writes a hash of `git diff --cached`
# to .git/embedded-review-ok when the review passes; any later staging change invalidates it.

cmd=$(jq -r '.tool_input.command // empty')
printf '%s' "$cmd" | grep -Eq '(^|[;&|[:space:]])git([[:space:]]+-C[[:space:]]+[^[:space:]]+)?[[:space:]]+commit([[:space:]]|$)' || exit 0

git_dir=$(git rev-parse --git-dir 2>/dev/null) || exit 0
staged=$(git diff --cached | shasum | cut -c1-40)
approved=$(cat "$git_dir/embedded-review-ok" 2>/dev/null)

[ -n "$approved" ] && [ "$staged" = "$approved" ] && exit 0

jq -n '{
  hookSpecificOutput: {
    hookEventName: "PreToolUse",
    permissionDecision: "deny",
    permissionDecisionReason: "The staged changes have not passed the embedded review. Invoke the embedded-review skill on the staged changes, fix any blocking findings, then commit again (do not use git commit -a)."
  }
}'
