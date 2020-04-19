set(MF_DIR "mcufont")
set(DECODER_DIR "${MF_DIR}/decoder")
set(FONTS_DIR "${MF_DIR}/fonts")

set(SRCS
	"${DECODER_DIR}/mf_encoding.c"
	"${DECODER_DIR}/mf_font.c"
	"${DECODER_DIR}/mf_justify.c"
	"${DECODER_DIR}/mf_kerning.c"
	"${DECODER_DIR}/mf_rlefont.c"
	"${DECODER_DIR}/mf_bwfont.c"
	"${DECODER_DIR}/mf_scaledfont.c"
	"${DECODER_DIR}/mf_wordwrap.c"
)
set(INCLUDE_DIRS
	"${DECODER_DIR}"
	"${FONTS_DIR}"
    "include"
)

idf_component_register(
	SRCS "${SRCS}"
	INCLUDE_DIRS "${INCLUDE_DIRS}"
)
