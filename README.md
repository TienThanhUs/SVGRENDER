# SVGRENDER

<div align="center">

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub Stars](https://img.shields.io/github/stars/TienThanhUs/SVGRENDER?style=social)](https://github.com/TienThanhUs/SVGRENDER/stargazers)
[![GitHub Issues](https://img.shields.io/github/issues/TienThanhUs/SVGRENDER)](https://github.com/TienThanhUs/SVGRENDER/issues)

A lightweight, high-performance SVG rendering library designed for flexibility and ease of integration.

</div>

---

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
- [Usage](#usage)
- [API Reference](#api-reference)
- [Contributing](#contributing)
- [License](#license)
- [Author](#author)

---

## Overview

**SVGRENDER** is a powerful SVG rendering engine that lets you parse, manipulate, and render Scalable Vector Graphics (SVG) files with ease. Whether you are building a design tool, a data visualization platform, or a web application that needs dynamic graphics, SVGRENDER provides the building blocks you need.

---

## ✨ Features

- 🖼️ **Full SVG Parsing** — Supports a wide range of SVG elements and attributes
- ⚡ **High Performance** — Optimised rendering pipeline for smooth output
- 🔧 **Extensible** — Plugin-friendly architecture for custom elements and renderers
- 📐 **Accurate Layout** — Precise geometry calculations respecting SVG coordinate systems
- 🌐 **Cross-platform** — Runs on major operating systems and environments
- 📦 **Zero dependencies** — Minimal footprint with no external runtime dependencies

---

## 🚀 Getting Started

### Prerequisites

Before you begin, make sure you have the following installed:

- **Node.js** v16 or higher *(if using the JavaScript build)*
- **Git** for cloning the repository

### Installation

#### Clone the repository

```bash
git clone https://github.com/TienThanhUs/SVGRENDER.git
cd SVGRENDER
```

#### Install dependencies *(if applicable)*

```bash
npm install
```

---

## 💡 Usage

```js
import { SVGRenderer } from './src/SVGRenderer';

const renderer = new SVGRenderer();
const svgContent = `<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">
  <circle cx="50" cy="50" r="40" fill="royalblue" />
</svg>`;

renderer.render(svgContent, document.getElementById('canvas'));
```

More examples can be found in the [`examples/`](examples/) directory.

---

## 📖 API Reference

| Method | Parameters | Description |
|--------|-----------|-------------|
| `render(svg, target)` | `svg: string`, `target: HTMLElement` | Parses and renders an SVG string into the target element |
| `parse(svg)` | `svg: string` | Parses an SVG string and returns an AST |
| `export(format)` | `format: 'png' \| 'jpg' \| 'svg'` | Exports the current canvas to the specified format |

> **Note:** Full API documentation will be available soon.

---

## 🤝 Contributing

Contributions are welcome and greatly appreciated!

1. **Fork** the repository
2. **Create** a feature branch: `git checkout -b feat/amazing-feature`
3. **Commit** your changes: `git commit -m "feat: add amazing feature"`
4. **Push** to the branch: `git push origin feat/amazing-feature`
5. **Open** a Pull Request

Please read [CONTRIBUTING.md](CONTRIBUTING.md) *(coming soon)* for details on the code of conduct and the contribution process.

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## 👤 Author

**Nguyen Tien Thanh**

- GitHub: [@TienThanhUs](https://github.com/TienThanhUs)

---

<div align="center">
  Made with ❤️ by <a href="https://github.com/TienThanhUs">Nguyen Tien Thanh</a>
</div>