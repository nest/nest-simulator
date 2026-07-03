/**
 * Non-stable version warning banner for the NEST Simulator documentation.
 *
 * Read the Docs' built-in "addons" version-warning notification does not
 * render correctly with the sphinx_material theme used here, so this script
 * is a workaround: it is loaded as a Read the Docs "Custom Script"
 * (Admin -> Settings -> Addons -> Custom Script), which injects it as a
 * <script> tag on every page of every version, including old versions that
 * are never rebuilt.
 *
 * Deploy: point the Custom Script setting at the built copy of this file on
 * a version that is rebuilt on every change (e.g.
 * https://nest-simulator.readthedocs.io/en/main/_static/js/rtd-version-banner.js),
 * so all versions always load the latest banner logic.
 *
 * Everything (styles included) lives in this one file because the Custom
 * Script setting only accepts a single script URL -- there is nowhere to
 * also register a separate stylesheet.
 *
 * See: https://docs.readthedocs.com/platform/latest/custom-script.html
 */
(function () {
  "use strict";

  if (document.querySelector(".rtd-version-banner")) {
    return;
  }

  var STABLE_DOCS_URL = "https://nest-simulator.readthedocs.io/en/stable/";
  var MAIN_DOCS_URL = "https://nest-simulator.readthedocs.io/en/main/";

  // Read the Docs URLs always follow /en/<version>/..., so the version slug
  // is read straight from the path. This avoids depending on any RTD
  // addons/meta-tag API, which may not be available if addons are broken
  // for this theme (the reason this workaround exists in the first place).
  function getVersionSlug() {
    var parts = window.location.pathname.split("/").filter(Boolean);
    var enIndex = parts.indexOf("en");
    var slug = enIndex !== -1 ? parts[enIndex + 1] : parts[0];
    if (!slug || !/^[a-zA-Z0-9_.-]+$/.test(slug)) {
      return null;
    }
    return slug;
  }

  var slug = getVersionSlug();
  if (!slug || slug === "stable") {
    return;
  }

  var isDevelopment = slug === "main" || slug === "latest";
  var bannerType = isDevelopment ? "development" : "outdated";
  var dismissKey = "rtd-banner-dismissed-" + bannerType;

  try {
    if (window.localStorage && window.localStorage.getItem(dismissKey) === "true") {
      return;
    }
  } catch (e) {
    // localStorage unavailable (e.g. privacy mode) -- fall through and show the banner.
  }

  function injectStyles() {
    var style = document.createElement("style");
    style.textContent =
      ".rtd-version-banner{position:fixed;left:0;right:0;bottom:0;z-index:10000;" +
      "display:flex;align-items:center;justify-content:center;gap:1rem;" +
      "padding:0.75rem 1.25rem;" +
      "padding-bottom:calc(0.75rem + env(safe-area-inset-bottom, 0px));" +
      "font-family:'Roboto','Helvetica Neue',Helvetica,Arial,sans-serif;" +
      "font-size:0.875rem;line-height:1.4;" +
      "box-shadow:0 -2px 6px rgba(0,0,0,0.15);}" +
      ".rtd-version-banner--development{background-color:#2196f3;color:#fff;}" +
      ".rtd-version-banner--outdated{background-color:#ff9800;color:#212121;}" +
      ".rtd-version-banner__text{max-width:60rem;}" +
      ".rtd-version-banner__text a{color:inherit;text-decoration:underline;font-weight:600;}" +
      ".rtd-version-banner__text code{background:rgba(0,0,0,0.12);padding:0.1em 0.35em;" +
      "border-radius:3px;font-size:0.9em;}" +
      ".rtd-version-banner__close{flex-shrink:0;background:transparent;border:none;" +
      "color:inherit;font-size:1.25rem;line-height:1;cursor:pointer;padding:0.25rem 0.5rem;" +
      "opacity:0.8;}" +
      ".rtd-version-banner__close:hover{opacity:1;}" +
      "@media (max-width:600px){.rtd-version-banner{flex-direction:column;gap:0.5rem;" +
      "text-align:center;}}";
    document.head.appendChild(style);
  }

  function appendStrong(parent, txt) {
    var el = document.createElement("strong");
    el.textContent = txt;
    parent.appendChild(el);
  }

  function appendCode(parent, txt) {
    var el = document.createElement("code");
    el.textContent = txt;
    parent.appendChild(el);
  }

  function appendLink(parent, href, txt) {
    var el = document.createElement("a");
    el.href = href;
    el.textContent = txt;
    parent.appendChild(el);
  }

  function buildBanner() {
    var banner = document.createElement("div");
    banner.className = "rtd-version-banner rtd-version-banner--" + bannerType;
    banner.setAttribute("role", "note");

    var text = document.createElement("span");
    text.className = "rtd-version-banner__text";

    if (isDevelopment) {
      text.appendChild(document.createTextNode("You are viewing the "));
      appendStrong(text, "latest development");
      text.appendChild(document.createTextNode(" version of the NEST documentation, built from "));
      appendCode(text, "main");
      text.appendChild(document.createTextNode(" branch. It may describe unreleased or unstable features. "));
      appendLink(text, STABLE_DOCS_URL, "View the stable release docs");
      text.appendChild(document.createTextNode("."));
    } else {
      text.appendChild(document.createTextNode("You are viewing an "));
      appendStrong(text, "outdated or non-stable");
      text.appendChild(document.createTextNode(" version of the NEST documentation ("));
      appendCode(text, slug);
      text.appendChild(document.createTextNode("). "));
      appendLink(text, STABLE_DOCS_URL, "View the latest stable release");
    }

    var closeBtn = document.createElement("button");
    closeBtn.type = "button";
    closeBtn.className = "rtd-version-banner__close";
    closeBtn.setAttribute("aria-label", "Dismiss");
    closeBtn.textContent = "×";
    closeBtn.addEventListener("click", function () {
      banner.remove();
      try {
        window.localStorage.setItem(dismissKey, "true");
      } catch (e) {
        // localStorage unavailable -- banner still closes for this page view.
      }
    });

    banner.appendChild(text);
    banner.appendChild(closeBtn);
    return banner;
  }

  function show() {
    injectStyles();
    document.body.appendChild(buildBanner());
  }

  if (document.body) {
    show();
  } else {
    document.addEventListener("DOMContentLoaded", show);
  }
})();
