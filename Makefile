include $(TOPDIR)/rules.mk

PKG_NAME:=udp-broadcast
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/udp-broadcast-$(PKG_VERSION)
PKG_SOURCE:=udp-broadcast-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/udp-broadcast
PKG_MD5SUM:=9b7dc52656f5cbec846a7ba3299f73bd
PKG_CAT:=zcat

include $(INCLUDE_DIR)/package.mk

define Package/udp-broadcast
  SECTION:=base
  CATEGORY:=Network
  DEFAULT:=y
  TITLE:=UDP broadcast utility
  DESCRIPTION:=UDP broadcast utility\\\
    send and recive udp broadcast\\\
    form a larger network.
  URL:=http://www.freifunk-jena.de/
endef

define Build/Configure
  $(call Build/Configure/Default,--with-linux-headers=$(LINUX_DIR))
endef

define Package/bridge/install
        $(INSTALL_DIR) $(1)/usr/sbin
        $(INSTALL_BIN) $(PKG_BUILD_DIR)/broadcast $(1)/usr/sbin/
        $(INSTALL_BIN) $(PKG_BUILD_DIR)/listener $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,udp-broadcast))
