# 提交规范

本文档描述 Git 提交信息的书写规范。

---

## 提交信息格式

```
<type>: <subject>

<body>

<footer>
```

## Type

| 类型 | 说明 |
|------|------|
| `feat` | 新功能 |
| `fix` | Bug 修复 |
| `docs` | 仅文档变更 |
| `style` | 代码格式调整，不影响功能 |
| `refactor` | 重构，既不新增功能也不修复 Bug |
| `perf` | 性能优化 |
| `test` | 测试相关 |
| `chore` | 构建脚本、工具链等杂项 |

## Subject

- 使用祈使句，首字母不大写，末尾不加句号
- 不超过 50 个字符
- 示例：`fix: correct FSMC NE4 chip select polarity`

## Body

- 说明变更动机与实现细节
- 每行不超过 72 个字符

## Footer

- 关联 Issue：`Closes #123`
- 破坏性变更：`BREAKING CHANGE: ...`

## 示例

```
feat: add XPT2046 touch screen driver

Implement SPI2-based touch controller initialization
and position reading. Touch interrupt uses PC5.

Closes #7
```
