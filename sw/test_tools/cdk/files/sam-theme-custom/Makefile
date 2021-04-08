include $(TOPDIR)/rules.mk

PKG_NAME:=sam-theme-custom
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

LANGS := en de es fr it ja pt zh-hk zh-tw
PO2LMO := $(BUILD_DIR)/luci-0.11.1/build/po2lmo

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/po
	mkdir -p $(PKG_BUILD_DIR)/lmo
	$(CP) ./po/* $(PKG_BUILD_DIR)/po/
endef

define Build/Compile
	$(foreach lang, $(LANGS), \
		$(PO2LMO) $(PKG_BUILD_DIR)/po/sam-ui.$(lang).po $(PKG_BUILD_DIR)/lmo/sam-ui.$(lang).lmo ; \
		touch $(PKG_BUILD_DIR)/lmo/sam-ui.$(lang).lmo ; \
	)
endef

define Package/luci-theme-sam-custom
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=4. Themes
  TITLE:=AllPlay Smart Audio Module Custom Theme
  DEPENDS:=+luci-theme-base
endef

define Package/luci-theme-sam-custom/install
	$(CP) ./files/root/* $(1)

	$(INSTALL_DIR) $(1)/CONTROL
	$(CP) -a ./files/ipkg/* $(1)/CONTROL/

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(CP) $(PKG_BUILD_DIR)/lmo/* $(1)/usr/lib/lua/luci/i18n/

	$(INSTALL_DIR) $(1)/www
	$(CP) ./files/htdocs/* $(1)/www/
endef

$(eval $(call BuildPackage,luci-theme-sam-custom))

