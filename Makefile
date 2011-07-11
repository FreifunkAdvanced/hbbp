include $(TOPDIR)/rules.mk

PKG_NAME:=udp-broadcast
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/udp-broadcast
  SECTION:=base
  CATEGORY:=Network
  DEFAULT:=n
  TITLE:=UDP broadcast utility
  URL:=http://www.freifunk-jena.de/
endef

define Package/udp-broadcast/description
	Send udp based broadcast messages and process them using the included demon.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)  \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/udp-broadcast/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/broadcaster $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/listener $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,udp-broadcast))
