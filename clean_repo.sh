#!/bin/bash
echo "开始清理大文件..."

# 列出要删除的文件模式
PATTERNS=(
  ".vs/EScapeGame/v17/Browse.VC.db"
  ".vs/EScapeGame/v17/*.jpch"
  ".vs/EScapeGame/v17/Solution.VC.db"
  ".vs/EScapeGame/CopilotIndices/*"
  "*.VC.db"
  "*.jpch"
)

# 使用git filter-repo清理
for pattern in "${PATTERNS[@]}"; do
  echo "清理模式: $pattern"
  git filter-repo --path-glob "$pattern" --force --quiet
done

echo "清理完成！"
