<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

  <xsl:output method="html"/>

  <xsl:template match="/">
    <html>
      <head>
        <title>Devil's Pie Matcher/Action Documentation</title>
      </head>
      <body>
        <h1>Devil's Pie Matcher/Action Documentation</h1>
        <xsl:apply-templates/>
      </body>
    </html>
  </xsl:template>

  <xsl:template match="matchers">
    <h2>Matchers</h2>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="actions">
    <h2>Actions</h2>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="matcher|action">
    <h3><xsl:value-of select="@name"/></h3>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="property">
    <p>
      <tt><xsl:value-of select="@name"/> (<xsl:value-of select="@type"/>)</tt>: <xsl:value-of select="@nick"/>
      <br/>
      <xsl:apply-templates/>
    </p>
  </xsl:template>

</xsl:stylesheet>
