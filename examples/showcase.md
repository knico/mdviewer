# mdviewer · Markdown 预览

离线阅读，专注内容。支持 **加粗**、*斜体*、~~删除线~~ 与 `行内代码`。

## 功能清单

- [x] 中文与 Unicode
- [x] 代码高亮与表格
- [x] 数学公式和 Mermaid

| 功能 | 渲染方式 |
|---|---|
| 公式 | KaTeX |
| 图表 | Mermaid |

> 本地资源，无需在线 CDN。

```javascript
const message = 'Hello, mdviewer';
console.log(message);
```

$$
E = mc^2
$$

```mermaid
flowchart LR
    A[编辑 Markdown] --> B[自动刷新]
    B --> C[本地预览]
```

脚注示例[^note]。

[^note]: Markdown 扩展以 README 中列出的支持范围为准。
