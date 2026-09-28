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
 * The newest release is exempt from the banner. Which version that is comes
 * from the GitHub releases API at run time, so no code change is needed when
 * a new version is released. When that lookup fails there is no way to tell
 * the current release from an outdated one, so no banner is shown at all.
 *
 * See: https://docs.readthedocs.com/platform/latest/custom-script.html
 */
(function () {
  "use strict";

  if (document.querySelector(".rtd-version-banner")) {
    return;
  }

  var STABLE_DOCS_URL = "https://nest-simulator.readthedocs.io/en/stable/";

  var RELEASE_API_URL = "https://api.github.com/repos/nest/nest-simulator/releases/latest";
  var RELEASE_CACHE_KEY = "rtd-banner-latest-release";
  // GitHub allows 60 unauthenticated requests per hour and IP, which a shared
  // institute network would exhaust in one browsing session if every page view
  // hit the API. Caching keeps it to a handful of requests per browser per day;
  // the trade-off is that a freshly released version may keep showing the
  // banner until the cached answer expires.
  var RELEASE_CACHE_TTL_MS = 6 * 60 * 60 * 1000;
  var RELEASE_FETCH_TIMEOUT_MS = 4000;

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
  // Release candidates stay published on Read the Docs (v3.10_rc1 etc.). They
  // are never the current release, but during a release cycle they are ahead
  // of it rather than behind, so they get their own wording.
  var isPreRelease = /[._-]rc\d*$/i.test(slug);
  var bannerType = isDevelopment ? "development" : isPreRelease ? "prerelease" : "outdated";
  var dismissKey = "rtd-banner-dismissed-" + bannerType;

  function isDismissed() {
    try {
      return !!window.localStorage && window.localStorage.getItem(dismissKey) === "true";
    } catch (e) {
      // localStorage unavailable (e.g. privacy mode) -- show the banner.
      return false;
    }
  }

  // RTD version slugs and git tags are both "v3.10" today, but normalise both
  // sides so the comparison survives either one dropping the "v".
  function normalizeVersion(version) {
    return String(version).trim().toLowerCase().replace(/^v/, "");
  }

  function readCachedRelease() {
    try {
      var raw = window.localStorage.getItem(RELEASE_CACHE_KEY);
      if (!raw) {
        return null;
      }
      var entry = JSON.parse(raw);
      if (!entry || typeof entry.tag !== "string" || typeof entry.time !== "number") {
        return null;
      }
      if (Date.now() - entry.time > RELEASE_CACHE_TTL_MS) {
        return null;
      }
      return entry.tag;
    } catch (e) {
      // Unreadable or malformed cache -- fall through to the API.
      return null;
    }
  }

  function writeCachedRelease(tag) {
    try {
      window.localStorage.setItem(
        RELEASE_CACHE_KEY,
        JSON.stringify({ tag: tag, time: Date.now() })
      );
    } catch (e) {
      // The cache is an optimisation only -- ignore failures.
    }
  }

  function fetchLatestRelease() {
    var options = {
      headers: {
        Accept: "application/vnd.github+json",
        "X-GitHub-Api-Version": "2022-11-28",
      },
    };
    // Without a timeout a hanging request would leave the banner decision
    // pending forever.
    if (typeof AbortSignal !== "undefined" && typeof AbortSignal.timeout === "function") {
      options.signal = AbortSignal.timeout(RELEASE_FETCH_TIMEOUT_MS);
    }

    return fetch(RELEASE_API_URL, options)
      .then(function (response) {
        if (!response.ok) {
          throw new Error("GitHub API error " + response.status);
        }
        return response.json();
      })
      .then(function (release) {
        if (!release || typeof release.tag_name !== "string" || !release.tag_name) {
          throw new Error("GitHub API response has no tag_name");
        }
        return release.tag_name;
      });
  }

  // Only calls back when the current release is known. Without that version
  // an outdated slug cannot be told apart from the current one, so a failed
  // lookup silently skips the banner rather than risk marking the current
  // release outdated.
  function getLatestRelease(callback) {
    var cached = readCachedRelease();
    if (cached) {
      callback(cached);
      return;
    }
    if (typeof fetch !== "function") {
      return;
    }
    fetchLatestRelease().then(
      function (tag) {
        writeCachedRelease(tag);
        callback(tag);
      },
      function () {
        // Unreachable, rate-limited or malformed -- leave the banner off.
      }
    );
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
      ".rtd-version-banner--development{background-color:#16c8ff;color:#fff;}" +
      ".rtd-version-banner--outdated,.rtd-version-banner--prerelease" +
      "{background-color:#ffdd27;color:#212121;}" +
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
    } else if (isPreRelease) {
      text.appendChild(document.createTextNode("You are viewing a "));
      appendStrong(text, "release candidate");
      text.appendChild(document.createTextNode(" version of the NEST documentation ("));
      appendCode(text, slug);
      text.appendChild(document.createTextNode("). "));
      appendLink(text, STABLE_DOCS_URL, "View the latest stable release");
    } else {
      text.appendChild(document.createTextNode("You are viewing an "));
      appendStrong(text, "outdated");
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
    // The release lookup is asynchronous, so re-check: another copy of the
    // script may have injected a banner while the request was in flight.
    if (document.querySelector(".rtd-version-banner")) {
      return;
    }
    injectStyles();
    document.body.appendChild(buildBanner());
  }

  function render() {
    if (document.body) {
      show();
    } else {
      document.addEventListener("DOMContentLoaded", show);
    }
  }

  // Checked before the release lookup so a dismissed banner costs no request.
  if (isDismissed()) {
    return;
  }

  if (isDevelopment || isPreRelease) {
    // Neither can ever be the current release, so never wait on the network.
    render();
    return;
  }

  getLatestRelease(function (latest) {
    if (normalizeVersion(slug) === normalizeVersion(latest)) {
      return;
    }
    render();
  });
})();
