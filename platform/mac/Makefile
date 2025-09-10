###
### AquaSKK
###

DEVELOPMENT_TEAM?="" # required to sign a release build for notarization. try make pkg DEVELOPMENT_TEAM=XXXX
APPSTORECONNECT_USERNAME?="" # required to submit a release build for notarization. try make notarize APPSTORECONNECT_USERNAME=XXXX

ifneq (,$(filter pkg notarize,$(MAKECMDGOALS)))
	RELEASE_BUILD_OPTS = CODE_SIGN_INJECT_BASE_ENTITLEMENTS=NO CODE_SIGN_IDENTITY="Developer ID Application" DEVELOPMENT_TEAM=$(DEVELOPMENT_TEAM)
endif

TARGET	= AquaSKK.app
DEST	= /Library/Input\ Methods
WORKDIR	= /tmp/AquaSKK
XCODE	= xcodebuild -project proj/AquaSKK.xcodeproj
DEBUG_TARGET	= proj/build/Debug/$(TARGET)
RELEASE_TARGET	= proj/build/Release/$(TARGET)
DEBUG_BUILD	= $(XCODE) -configuration Debug
RELEASE_BUILD	= $(XCODE) -configuration Release $(RELEASE_BUILD_OPTS)
HDIMAGE	= hdiutil create -ov -srcfolder $(WORKDIR)
TODAY	= `date +%Y%m%d`

PKG_ROOT = $(WORKDIR)/root
PKG_ID = org.codefirst.aquaskk.pkg
PKG_VERSION = 4.2.5
PKG_SIGN_ID = "Developer ID"

ifeq ($(MAKECMDGOALS), install)
	COPY_SOURCE	= $(DEBUG_TARGET)
else
	COPY_SOURCE	= $(RELEASE_TARGET)
endif

all: debug

clean:
	$(DEBUG_BUILD) clean
	$(RELEASE_BUILD) clean

install: debug copy kill

install-rc: release copy kill

copy:
	sudo rm -rf $(DEST)/$(TARGET)
	sudo cp -r $(COPY_SOURCE) $(DEST)
	sync; sync; sync

test:
	cd ../../src/engine/tests; make test;

debug:
	$(DEBUG_BUILD) build

release:
	$(RELEASE_BUILD) build

pkg: release
	rm -fr AquaSKK.pkg
	mkdir -p $(PKG_ROOT)/$(DEST)
	cp -r $(RELEASE_TARGET) $(PKG_ROOT)/$(DEST)
	pkgbuild --root $(PKG_ROOT) --component-plist pkg/aquaskk.plist --identifier $(PKG_ID) --version $(PKG_VERSION) $(WORKDIR)/aquaskk-pkg.pkg
	productbuild --distribution pkg/distribution.xml --package-path $(WORKDIR) --resources 'proj/contents' $(WORKDIR)/aquaskk-dist.pkg
	productsign --sign $(PKG_SIGN_ID) $(WORKDIR)/aquaskk-dist.pkg AquaSKK.pkg

notarize: pkg
	xcrun notarytool submit --wait AquaSKK.pkg --apple-id $(APPSTORECONNECT_USERNAME) --team-id $(DEVELOPMENT_TEAM)
	xcrun stapler staple AquaSKK.pkg

dmg: pkg
	rm -fr $(WORKDIR)
	mkdir $(WORKDIR)
	mv AquaSKK.pkg $(WORKDIR)
	$(HDIMAGE) /tmp/AquaSKK-$(TODAY).dmg

kill:
	@pid=`ps -A|awk '/[A]quaSKK.app/ { print $$1; }'`; \
	if [[ "$$pid" -ne "" ]]; then \
		kill $$pid; \
	fi
