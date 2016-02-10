<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text"/>
	<xsl:template match="/">
		<xsl:apply-templates select="xar/toc/file"/>
	</xsl:template>

	<xsl:template match="file">
		<xsl:value-of select="name"/>
		<xsl:text disable-output-escaping="yes">&#032;</xsl:text>
		<xsl:value-of select="data/offset"/>
		<xsl:text disable-output-escaping="yes">&#010;</xsl:text>
	</xsl:template>
</xsl:stylesheet>
