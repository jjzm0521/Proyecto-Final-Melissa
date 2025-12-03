import markdown
import codecs

# Read Markdown file
input_file = "smart_lock_documentation.md"
output_file = "smart_lock_documentation.html"

with codecs.open(input_file, mode="r", encoding="utf-8") as f:
    text = f.read()

# Convert to HTML
html = markdown.markdown(text, extensions=['tables', 'fenced_code'])

# Add CSS and Mermaid JS
full_html = f"""
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<title>Documentación Smart Lock</title>
<style>
    body {{ font-family: sans-serif; line-height: 1.6; max-width: 800px; margin: 0 auto; padding: 20px; }}
    h1, h2, h3 {{ color: #333; }}
    code {{ background-color: #f4f4f4; padding: 2px 5px; border-radius: 3px; }}
    pre {{ background-color: #f4f4f4; padding: 10px; border-radius: 5px; overflow-x: auto; }}
    table {{ border-collapse: collapse; width: 100%; margin-bottom: 20px; }}
    th, td {{ border: 1px solid #ddd; padding: 8px; text-align: left; }}
    th {{ background-color: #f2f2f2; }}
    .mermaid {{ text-align: center; margin: 20px 0; }}
</style>
<script type="module">
    import mermaid from 'https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.esm.min.mjs';
    mermaid.initialize({{ startOnLoad: true }});
</script>
</head>
<body>
{html}
</body>
</html>
"""

# Fix Mermaid blocks (Markdown converts them to <pre><code>, we need <div class="mermaid">)
full_html = full_html.replace('<pre><code class="language-mermaid">', '<div class="mermaid">').replace('</code></pre>', '</div>')

with codecs.open(output_file, mode="w", encoding="utf-8") as f:
    f.write(full_html)

print(f"Successfully created {output_file}")
